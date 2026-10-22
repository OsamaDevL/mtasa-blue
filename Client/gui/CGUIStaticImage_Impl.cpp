/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        gui/CGUIStaticImage_Impl.cpp
 *  PURPOSE:     Static image widget class
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"

#define CGUISTATICIMAGE_NAME "CGUI/StaticImage"

CGUIStaticImage_Impl::CGUIStaticImage_Impl(CGUI_Impl* pGUI, CGUIElement* pParent)
{
    // Initialize
#ifdef MTA_USE_CEGUI_NEXT
    m_pImage = NULL;
#else
    m_pImagesetManager = pGUI->GetImageSetManager();
    m_pImageset = NULL;
    m_pImage = NULL;
#endif
    m_pGUI = pGUI;
    SetManager(pGUI);
    m_pTexture = NULL;
    m_bCreatedTexture = false;

    // Get an unique identifier for CEGUI
    char szUnique[CGUI_CHAR_SIZE];
    pGUI->GetUniqueName(szUnique);

    // Create the control and set default properties
    m_pWindow = pGUI->GetWindowManager()->createWindow(pGUI->ResolveWindowType(CGUISTATICIMAGE_NAME, pParent), szUnique);
    m_pWindow->setDestroyedByParent(false);
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setSize(CEGUI::USize(CEGUI::UDim(1.0f, 0.0f), CEGUI::UDim(1.0f, 0.0f)));
    m_pWindow->setProperty("BackgroundEnabled", "False");
#else
    m_pWindow->setRect(CEGUI::Relative, CEGUI::Rect(0.0f, 0.0f, 1.0f, 1.0f));
    reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->setBackgroundEnabled(false);
#endif

    // Store the pointer to this CGUI element in the CEGUI element
    m_pWindow->setUserData(reinterpret_cast<void*>(this));

    AddEvents();

    // If a parent is specified, add it to it's children list, if not, add it as a child to the pManager
    if (pParent)
    {
        SetParent(pParent);
    }
    else
    {
        pGUI->AddChild(this);
        SetParent(NULL);
    }
}

CGUIStaticImage_Impl::~CGUIStaticImage_Impl()
{
    // Clear the image
    Clear();

    DestroyElement();
}

bool CGUIStaticImage_Impl::LoadFromFile(const char* szFilename)
{
    // Load texture
    if (!m_pTexture)
    {
        m_pTexture = new CGUITexture_Impl(m_pGUI);
        m_bCreatedTexture = true;
    }

    if (!m_pTexture->LoadFromFile(szFilename))
        return false;

    // Load image
    return LoadFromTexture(m_pTexture);
}

bool CGUIStaticImage_Impl::LoadFromTexture(CGUITexture* pTexture)
{
    if (!pTexture)
        return false;

#ifdef MTA_USE_CEGUI_NEXT
    if (m_pTexture && pTexture != m_pTexture)
    {
        m_pWindow->setProperty("Image", "");
        if (!m_strImageName.empty() && CEGUI::ImageManager::getSingleton().isDefined(m_strImageName))
        {
            CEGUI::ImageManager::getSingleton().destroy(m_strImageName);
            m_strImageName.clear();
        }
        m_pImage = nullptr;
        if (m_bCreatedTexture)
        {
            delete m_pTexture;
            m_pTexture = NULL;
            m_bCreatedTexture = false;
        }
    }

    m_pTexture = (CGUITexture_Impl*)pTexture;

    // Get CEGUI texture
    CEGUI::Texture* pCEGUITexture = m_pTexture->GetTexture();
    if (!pCEGUITexture)
        return false;

    char szUnique[CGUI_CHAR_SIZE];
    m_pGUI->GetUniqueName(szUnique);

    try
    {
        if (!m_strImageName.empty() && CEGUI::ImageManager::getSingleton().isDefined(m_strImageName))
        {
            CEGUI::ImageManager::getSingleton().destroy(m_strImageName);
            m_strImageName.clear();
        }

        while (CEGUI::ImageManager::getSingleton().isDefined(szUnique))
            m_pGUI->GetUniqueName(szUnique);

        m_strImageName = szUnique;
        CEGUI::BasicImage* pBasicImage = static_cast<CEGUI::BasicImage*>(&CEGUI::ImageManager::getSingleton().create("BasicImage", m_strImageName));
        pBasicImage->setTexture(pCEGUITexture);
        pBasicImage->setArea(CEGUI::Rectf(CEGUI::Vector2f(0.0f, 0.0f), pCEGUITexture->getOriginalDataSize()));
        m_pImage = pBasicImage;

        m_pWindow->setProperty("Image", m_strImageName);
    }
    catch (const CEGUI::Exception& e)
    {
        (void)e;
        OutputDebugLine(SString("CGUIStaticImage_Impl::LoadFromTexture failed: %s", e.getMessage().c_str()));
        m_pWindow->setProperty("Image", "");
        m_pImage = nullptr;
        return false;
    }

    return true;
#else
    if (m_pTexture && pTexture != m_pTexture)
    {
        // The widget and its imageset keep references to the old texture, so
        // release them before the texture is replaced to avoid drawing from a
        // stale or freed texture.
        reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->setImage(nullptr);
        if (m_pImageset)
        {
            m_pImagesetManager->destroyImageset(m_pImageset);
            m_pImageset = nullptr;
        }
        m_pImage = nullptr;
        if (m_bCreatedTexture)
        {
            delete m_pTexture;
            m_pTexture = NULL;
            m_bCreatedTexture = false;
        }
    }

    m_pTexture = (CGUITexture_Impl*)pTexture;

    // Get CEGUI texture
    CEGUI::Texture* pCEGUITexture = m_pTexture->GetTexture();

    // Get an unique identifier for CEGUI for the imageset
    char szUnique[CGUI_CHAR_SIZE];
    m_pGUI->GetUniqueName(szUnique);

    try
    {
        // Clear any previously defined images so the widget cannot keep drawing
        // an image that a failed re-load has invalidated
        if (m_pImageset && m_pImage)
        {
            m_pImageset->undefineAllImages();
        }

        // Create an imageset
        if (!m_pImageset)
        {
            while (m_pImagesetManager->isImagesetPresent(szUnique))
                m_pGUI->GetUniqueName(szUnique);
            m_pImageset = m_pImagesetManager->createImageset(szUnique, pCEGUITexture, true);
        }

        // Get an unique identifier for CEGUI for the image
        m_pGUI->GetUniqueName(szUnique);

        // Define an image and get its pointer
        m_pImageset->defineImage(szUnique, CEGUI::Point(0, 0), CEGUI::Size(pCEGUITexture->getWidth(), pCEGUITexture->getHeight()), CEGUI::Point(0, 0));
        m_pImage = &m_pImageset->getImage(szUnique);

        // Set the image just loaded as the image to be drawn for the widget
        reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->setImage(m_pImage);
    }
    catch (const CEGUI::Exception& e)
    {
        OutputDebugLine(SString("CGUIStaticImage_Impl::LoadFromTexture failed: %s", e.getMessage().c_str()));
        // The imageset may have been cleared before the failure, so stop the
        // widget from drawing an image that no longer exists.
        reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->setImage(nullptr);
        m_pImage = nullptr;
        return false;
    }

    // Success
    return true;
#endif
}

void CGUIStaticImage_Impl::Clear()
{
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setProperty("Image", "");
    if (!m_strImageName.empty() && CEGUI::ImageManager::getSingleton().isDefined(m_strImageName))
    {
        CEGUI::ImageManager::getSingleton().destroy(m_strImageName);
        m_strImageName.clear();
    }
    m_pImage = nullptr;

    if (m_bCreatedTexture)
    {
        delete m_pTexture;
        m_pTexture = nullptr;
        m_bCreatedTexture = false;
    }
#else
    // Stop the control from using it
    reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->setImage(NULL);

    // Kill the images
    if (m_pImageset)
    {
        m_pImageset->undefineAllImages();
        m_pImagesetManager->destroyImageset(m_pImageset);
        m_pImage = NULL;
        m_pImageset = NULL;
    }

    // The imageset does not own the texture, so a texture created by this widget
    // must be released here even if the imageset never got created.
    if (m_bCreatedTexture)
    {
        delete m_pTexture;
        m_pTexture = nullptr;
        m_bCreatedTexture = false;
    }
#endif
}

bool CGUIStaticImage_Impl::GetNativeSize(CVector2D& vecSize)
{
    if (m_pTexture)
    {
        if (m_pTexture->GetTexture())
        {
#ifdef MTA_USE_CEGUI_NEXT
            vecSize.fX = m_pTexture->GetTexture()->getOriginalDataSize().d_width;
            vecSize.fY = m_pTexture->GetTexture()->getOriginalDataSize().d_height;
#else
            vecSize.fX = m_pTexture->GetTexture()->getWidth();
            vecSize.fY = m_pTexture->GetTexture()->getHeight();
#endif
            return true;
        }
    }
    return false;
}

void CGUIStaticImage_Impl::SetFrameEnabled(bool bFrameEnabled)
{
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setProperty("FrameEnabled", bFrameEnabled ? "True" : "False");
#else
    reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->setFrameEnabled(bFrameEnabled);
#endif
}

bool CGUIStaticImage_Impl::IsFrameEnabled()
{
#ifdef MTA_USE_CEGUI_NEXT
    return m_pWindow->getProperty("FrameEnabled") == "True";
#else
    return reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->isFrameEnabled();
#endif
}

CEGUI::Image* CGUIStaticImage_Impl::GetDirectImage()
{
#ifdef MTA_USE_CEGUI_NEXT
    return const_cast<CEGUI::Image*>(m_pImage);
#else
    return const_cast<CEGUI::Image*>(reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->getImage());
#endif
}

void CGUIStaticImage_Impl::Render()
{
#ifdef MTA_USE_CEGUI_NEXT
    return m_pWindow->render();
#else
    return reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->render();
#endif
}

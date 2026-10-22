/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        gui/CGUIWebBrowser_Impl.cpp
 *  PURPOSE:     WebBrowser widget class
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/
#include "StdInc.h"
#include <core/CWebViewInterface.h>

CGUIWebBrowser_Impl::CGUIWebBrowser_Impl(CGUI_Impl* pGUI, CGUIElement* pParent)
{
    // Initialize
#ifndef MTA_USE_CEGUI_NEXT
    m_pImagesetManager = pGUI->GetImageSetManager();
    m_pImageset = nullptr;
    m_pTexture = nullptr;
#endif
    m_pImage = nullptr;
    m_pGUI = pGUI;
    SetManager(pGUI);
    m_pWebView = nullptr;

    // Get an unique identifier for CEGUI
    char szUnique[CGUI_CHAR_SIZE];
    pGUI->GetUniqueName(szUnique);

    // Create the control and set default properties
    m_pWindow = pGUI->GetWindowManager()->createWindow(pGUI->ResolveWindowType(CGUIWEBBROWSER_NAME, pParent), szUnique);
    m_pWindow->setDestroyedByParent(false);
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setArea(CEGUI::URect(CEGUI::UDim(0.0f, 0.0f), CEGUI::UDim(0.0f, 0.0f), CEGUI::UDim(1.0f, 0.0f), CEGUI::UDim(1.0f, 0.0f)));
    m_pWindow->setProperty("BackgroundEnabled", "False");
#else
    m_pWindow->setRect(CEGUI::Relative, CEGUI::Rect(0.0f, 0.0f, 1.0f, 1.0f));
    reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->setBackgroundEnabled(false);
#endif

    // Store the pointer to this CGUI element in the CEGUI element
    m_pWindow->setUserData(reinterpret_cast<void*>(this));

    AddEvents();

    // Apply browser events
    m_pWindow->subscribeEvent(CEGUI::Window::EventMouseButtonDown, CEGUI::Event::Subscriber(&CGUIWebBrowser_Impl::Event_MouseButtonDown, this));
    m_pWindow->subscribeEvent(CEGUI::Window::EventMouseButtonUp, CEGUI::Event::Subscriber(&CGUIWebBrowser_Impl::Event_MouseButtonUp, this));
    m_pWindow->subscribeEvent(CEGUI::Window::EventMouseDoubleClick, CEGUI::Event::Subscriber(&CGUIWebBrowser_Impl::Event_MouseDoubleClick, this));
    m_pWindow->subscribeEvent(CEGUI::Window::EventMouseMove, CEGUI::Event::Subscriber(&CGUIWebBrowser_Impl::Event_MouseMove, this));
    m_pWindow->subscribeEvent(CEGUI::Window::EventMouseWheel, CEGUI::Event::Subscriber(&CGUIWebBrowser_Impl::Event_MouseWheel, this));
    m_pWindow->subscribeEvent(CEGUI::Window::EventActivated, CEGUI::Event::Subscriber(&CGUIWebBrowser_Impl::Event_Activated, this));
    m_pWindow->subscribeEvent(CEGUI::Window::EventDeactivated, CEGUI::Event::Subscriber(&CGUIWebBrowser_Impl::Event_Deactivated, this));

    // If a parent is specified, add it to it's children list, if not, add it as a child to the pManager
    if (pParent)
    {
        SetParent(pParent);
    }
    else
    {
        pGUI->AddChild(this);
        SetParent(nullptr);
    }
}

CGUIWebBrowser_Impl::~CGUIWebBrowser_Impl()
{
    Clear();

    DestroyElement();
}

void CGUIWebBrowser_Impl::Clear()
{
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setProperty("Image", "");
    if (!m_strImageName.empty() && CEGUI::ImageManager::getSingleton().isDefined(m_strImageName))
    {
        CEGUI::ImageManager::getSingleton().destroy(m_strImageName);
        m_strImageName.clear();
    }
    if (!m_strTextureName.empty())
    {
        m_pGUI->GetRenderer()->destroyTexture(m_strTextureName);
        m_strTextureName.clear();
    }
    m_pImage = nullptr;
#else
    // Stop the control from using it
    reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->setImage(nullptr);

    // Kill the images
    if (m_pImageset)
    {
        m_pImageset->undefineAllImages();
        m_pImagesetManager->destroyImageset(m_pImageset);
        m_pImage = nullptr;
        m_pImageset = nullptr;
    }

    // The imageset does not own the texture, so it must be released here
    if (m_pTexture)
    {
        delete m_pTexture;
        m_pTexture = nullptr;
    }
#endif
}

void CGUIWebBrowser_Impl::LoadFromWebView(CWebViewInterface* pWebView)
{
    m_pWebView = pWebView;

    if (!pWebView)
    {
        // No web view available (creation failed), so leave the widget blank
        Clear();
        return;
    }

    // Release any previous imageset and texture so a re-load cannot draw stale
    // content from an old web view or leak the old texture.
    Clear();

    try
    {
#ifdef MTA_USE_CEGUI_NEXT
        char szUnique[CGUI_CHAR_SIZE];
        m_pGUI->GetUniqueName(szUnique);
        m_strTextureName = szUnique;

        CEGUI::Texture& ceguiTexture = static_cast<CEGUI::Direct3D9Renderer*>(m_pGUI->GetRenderer())->createTexture(m_strTextureName, pWebView->GetTexture());

        m_pGUI->GetUniqueName(szUnique);
        m_strImageName = szUnique;

        CEGUI::BasicImage* pBasicImage = static_cast<CEGUI::BasicImage*>(&CEGUI::ImageManager::getSingleton().create("BasicImage", m_strImageName));
        pBasicImage->setTexture(&ceguiTexture);
        pBasicImage->setArea(CEGUI::Rectf(0.0f, 0.0f, pWebView->GetSize().fX, pWebView->GetSize().fY));
        m_pImage = pBasicImage;

        m_pWindow->setProperty("Image", m_strImageName);
#else
        m_pTexture = new CGUIWebBrowserTexture(m_pGUI->GetRenderer(), m_pWebView);

        // Get an unique identifier for CEGUI for the imageset
        char szUnique[CGUI_CHAR_SIZE];
        m_pGUI->GetUniqueName(szUnique);

        // Create an imageset
        if (!m_pImageset)
        {
            while (m_pImagesetManager->isImagesetPresent(szUnique))
                m_pGUI->GetUniqueName(szUnique);
            m_pImageset = m_pImagesetManager->createImageset(szUnique, m_pTexture, true);
        }

        // Get an unique identifier for CEGUI for the image
        m_pGUI->GetUniqueName(szUnique);

        // Define an image and get its pointer
        m_pImageset->defineImage(szUnique, CEGUI::Point(0, 0), CEGUI::Size(m_pTexture->getWidth(), m_pTexture->getHeight()), CEGUI::Point(0, 0));
        m_pImage = const_cast<CEGUI::Image*>(
            &m_pImageset->getImage(szUnique));  // const_cast here is a huge hack, but is okay here since all images generated here are unique

        // Set the image just loaded as the image to be drawn for the widget
        reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->setImage(m_pImage);
#endif
    }
    catch (const CEGUI::Exception& e)
    {
        (void)e;
        OutputDebugLine(SString("CGUIWebBrowser_Impl::LoadFromWebView failed: %s", e.getMessage().c_str()));
        // Release any partially created imageset or texture and leave the widget blank.
        Clear();
    }
}

void CGUIWebBrowser_Impl::SetFrameEnabled(bool bFrameEnabled)
{
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setProperty("FrameEnabled", bFrameEnabled ? "True" : "False");
#else
    reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->setFrameEnabled(bFrameEnabled);
#endif
}

bool CGUIWebBrowser_Impl::IsFrameEnabled()
{
#ifdef MTA_USE_CEGUI_NEXT
    return m_pWindow->getProperty("FrameEnabled") == "True";
#else
    return reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->isFrameEnabled();
#endif
}

CEGUI::Image* CGUIWebBrowser_Impl::GetDirectImage()
{
#ifdef MTA_USE_CEGUI_NEXT
    return m_pImage;
#else
    return const_cast<CEGUI::Image*>(reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->getImage());
#endif
}

void CGUIWebBrowser_Impl::Render()
{
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->render();
#else
    return reinterpret_cast<CEGUI::StaticImage*>(m_pWindow)->render();
#endif
}

bool CGUIWebBrowser_Impl::HasInputFocus()
{
    if (!m_pWebView)
        return false;
    return m_pWebView->HasInputFocus();
}

void CGUIWebBrowser_Impl::SetSize(const CVector2D& vecSize, bool bRelative)
{
    // Call base class function
    CGUIElement_Impl::SetSize(vecSize, bRelative);
    auto absSize = CGUIElement_Impl::GetSize(false);

    // Update image area
    if (m_pImage)
    {
#ifdef MTA_USE_CEGUI_NEXT
        static_cast<CEGUI::BasicImage*>(m_pImage)->setArea(CEGUI::Rectf(0.0f, 0.0f, absSize.fX, absSize.fY));
#else
        m_pImage->setSourceTextureArea(CEGUI::Rect(0, 0, absSize.fX, absSize.fY));
#endif
    }

    // Resize underlying web view as well
    if (m_pWebView)
        m_pWebView->Resize(absSize);
}

bool CGUIWebBrowser_Impl::Event_MouseButtonDown(const CEGUI::EventArgs& e)
{
    if (!m_pWebView)
        return true;

    const CEGUI::MouseEventArgs& args = reinterpret_cast<const CEGUI::MouseEventArgs&>(e);

    if (args.button == CEGUI::MouseButton::LeftButton)
        m_pWebView->InjectMouseDown(eWebBrowserMouseButton::BROWSER_MOUSEBUTTON_LEFT, 1);
    else if (args.button == CEGUI::MouseButton::MiddleButton)
        m_pWebView->InjectMouseDown(eWebBrowserMouseButton::BROWSER_MOUSEBUTTON_MIDDLE, 1);
    else if (args.button == CEGUI::MouseButton::RightButton)
        m_pWebView->InjectMouseDown(eWebBrowserMouseButton::BROWSER_MOUSEBUTTON_RIGHT, 1);

    return true;
}

bool CGUIWebBrowser_Impl::Event_MouseButtonUp(const CEGUI::EventArgs& e)
{
    if (!m_pWebView)
        return true;

    const CEGUI::MouseEventArgs& args = reinterpret_cast<const CEGUI::MouseEventArgs&>(e);

    if (args.button == CEGUI::MouseButton::LeftButton)
        m_pWebView->InjectMouseUp(eWebBrowserMouseButton::BROWSER_MOUSEBUTTON_LEFT);
    else if (args.button == CEGUI::MouseButton::MiddleButton)
        m_pWebView->InjectMouseUp(eWebBrowserMouseButton::BROWSER_MOUSEBUTTON_MIDDLE);
    else if (args.button == CEGUI::MouseButton::RightButton)
        m_pWebView->InjectMouseUp(eWebBrowserMouseButton::BROWSER_MOUSEBUTTON_RIGHT);

    return true;
}

bool CGUIWebBrowser_Impl::Event_MouseDoubleClick(const CEGUI::EventArgs& e)
{
    if (!m_pWebView)
        return true;

    const CEGUI::MouseEventArgs& args = reinterpret_cast<const CEGUI::MouseEventArgs&>(e);

    if (args.button == CEGUI::MouseButton::LeftButton)
        m_pWebView->InjectMouseDown(eWebBrowserMouseButton::BROWSER_MOUSEBUTTON_LEFT, 2);
    else if (args.button == CEGUI::MouseButton::MiddleButton)
        m_pWebView->InjectMouseDown(eWebBrowserMouseButton::BROWSER_MOUSEBUTTON_MIDDLE, 2);
    else if (args.button == CEGUI::MouseButton::RightButton)
        m_pWebView->InjectMouseDown(eWebBrowserMouseButton::BROWSER_MOUSEBUTTON_RIGHT, 2);

    return true;
}

#ifdef MTA_USE_CEGUI_NEXT
    #include <CEGUI/CoordConverter.h>
#endif

bool CGUIWebBrowser_Impl::Event_MouseMove(const CEGUI::EventArgs& e)
{
    if (!m_pWebView)
        return true;

    const CEGUI::MouseEventArgs& args = reinterpret_cast<const CEGUI::MouseEventArgs&>(e);

#ifdef MTA_USE_CEGUI_NEXT
    float localX = CEGUI::CoordConverter::screenToWindowX(*m_pWindow, args.position.d_x);
    float localY = CEGUI::CoordConverter::screenToWindowY(*m_pWindow, args.position.d_y);
    m_pWebView->InjectMouseMove(static_cast<int>(localX), static_cast<int>(localY));
#else
    m_pWebView->InjectMouseMove((int)(args.position.d_x - m_pWindow->windowToScreenX(0.0f)), (int)(args.position.d_y - m_pWindow->windowToScreenY(0.0f)));
#endif
    return true;
}

bool CGUIWebBrowser_Impl::Event_MouseWheel(const CEGUI::EventArgs& e)
{
    if (!m_pWebView)
        return true;

    const CEGUI::MouseEventArgs& args = reinterpret_cast<const CEGUI::MouseEventArgs&>(e);

    m_pWebView->InjectMouseWheel((int)(args.wheelChange * 40), 0);
    return true;
}

bool CGUIWebBrowser_Impl::Event_Activated(const CEGUI::EventArgs& e)
{
    if (!m_pWebView)
        return true;
    m_pWebView->Focus(true);
    return true;
}

bool CGUIWebBrowser_Impl::Event_Deactivated(const CEGUI::EventArgs& e)
{
    if (!m_pWebView)
        return true;
    m_pWebView->Focus(false);
    return true;
}

#ifndef MTA_USE_CEGUI_NEXT
CGUIWebBrowserTexture::CGUIWebBrowserTexture(CEGUI::Renderer* pOwner, CWebViewInterface* pWebView) : CEGUI::DirectX9Texture(pOwner), m_pWebView(pWebView)
{
}

ushort CGUIWebBrowserTexture::getWidth() const
{
    return static_cast<ushort>(m_pWebView->GetSize().fX);
}

ushort CGUIWebBrowserTexture::getHeight() const
{
    return static_cast<ushort>(m_pWebView->GetSize().fY);
}

LPDIRECT3DTEXTURE9 CGUIWebBrowserTexture::getD3DTexture() const
{
    return m_pWebView->GetTexture();
}
#endif

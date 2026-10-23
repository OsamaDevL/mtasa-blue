/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        gui/CGUIElement_Impl.cpp
 *  PURPOSE:     Element (widget) base class
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"
#include <core/CCoreInterface.h>

#ifdef MTA_USE_CEGUI_NEXT
    #include <CEGUI/CoordConverter.h>
#endif

// Define no-drawing zones, a.k.a. the inside borders in the FrameWindow of BlueLook in pixels
// If something is drawn inside of these areas, the theme border is drawn on top of it
#define CGUI_NODRAW_LEFT   9.0f
#define CGUI_NODRAW_RIGHT  9.0f
#define CGUI_NODRAW_TOP    9.0f
#define CGUI_NODRAW_BOTTOM 9.0f

CGUIElement_Impl::CGUIElement_Impl()
{
    m_pData = NULL;
    m_pWindow = NULL;
    m_pParent = NULL;
    m_pManager = NULL;
    m_redrawHandle = CGUI_Impl::kInvalidRedrawHandle;
}

void CGUIElement_Impl::SetManager(CGUI_Impl* pManager)
{
    if (m_pManager == pManager)
        return;

    if (m_pManager && m_redrawHandle != CGUI_Impl::kInvalidRedrawHandle)
    {
        m_pManager->ReleaseRedrawHandle(m_redrawHandle);
        m_redrawHandle = CGUI_Impl::kInvalidRedrawHandle;
    }

    m_pManager = pManager;

    if (m_pManager)
    {
        m_redrawHandle = m_pManager->RegisterRedrawHandle(this);
    }
}

void CGUIElement_Impl::UnregisterFromRedrawQueue()
{
    if (m_pManager && m_redrawHandle != CGUI_Impl::kInvalidRedrawHandle)
    {
        m_pManager->RemoveFromRedrawQueue(this);
    }
}

void CGUIElement_Impl::DestroyElement()
{
    UnregisterFromRedrawQueue();

    if (m_pWindow)
    {
        // Clear pointer back to this
        m_pWindow->setUserData(NULL);

        if (m_pManager)
        {
            // Destroy the control
            m_pManager->GetWindowManager()->destroyWindow(m_pWindow);
        }
        m_pWindow = NULL;
    }

    // Destroy the properties list
    EmptyProperties();

    if (m_pManager && m_redrawHandle != CGUI_Impl::kInvalidRedrawHandle)
    {
        m_pManager->ReleaseRedrawHandle(m_redrawHandle);
    }

    m_redrawHandle = CGUI_Impl::kInvalidRedrawHandle;
    m_pParent = NULL;
    m_pData = NULL;
    m_pManager = NULL;
}

void CGUIElement_Impl::SetVisible(bool bVisible)
{
    m_pWindow->setVisible(bVisible);
}

bool CGUIElement_Impl::IsVisible()
{
    return m_pWindow->isVisible();
}

void CGUIElement_Impl::SetEnabled(bool bEnabled)
{
    m_pWindow->setEnabled(bEnabled);
    // m_pWindow->setZOrderingEnabled ( bEnabled );
}

bool CGUIElement_Impl::IsEnabled()
{
    return !m_pWindow->isDisabled();
}

void CGUIElement_Impl::SetZOrderingEnabled(bool bZOrderingEnabled)
{
    m_pWindow->setZOrderingEnabled(bZOrderingEnabled);
}

bool CGUIElement_Impl::IsZOrderingEnabled()
{
    return m_pWindow->isZOrderingEnabled();
}

void CGUIElement_Impl::BringToFront()
{
    m_pWindow->moveToFront();
}

void CGUIElement_Impl::MoveToBack()
{
    m_pWindow->moveToBack();
}

void CGUIElement_Impl::SetPosition(const CVector2D& Position, bool bRelative)
{
#ifdef MTA_USE_CEGUI_NEXT
    if (bRelative)
        m_pWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(Position.fX, 0.0f), CEGUI::UDim(Position.fY, 0.0f)));
    else
        m_pWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0f, Position.fX), CEGUI::UDim(0.0f, Position.fY)));
#else
    CEGUI::Point Temp = CEGUI::Point(Position.fX, Position.fY);

    if (bRelative)
        m_pWindow->setPosition(CEGUI::Relative, Temp);
    else
        m_pWindow->setPosition(CEGUI::Absolute, Temp);
#endif

    CorrectEdges();
}

CVector2D CGUIElement_Impl::GetPosition(bool bRelative)
{
#ifdef MTA_USE_CEGUI_NEXT
    if (bRelative)
    {
        CEGUI::Window* pParent = m_pWindow->getParent();
        if (pParent)
        {
            CEGUI::Sizef parentSize = pParent->getPixelSize();
            if (parentSize.d_width != 0.0f && parentSize.d_height != 0.0f)
            {
                CEGUI::Vector2f absPos = CEGUI::CoordConverter::asAbsolute(m_pWindow->getPosition(), parentSize);
                return CVector2D(absPos.d_x / parentSize.d_width, absPos.d_y / parentSize.d_height);
            }
        }
        return CVector2D(m_pWindow->getPosition().d_x.d_scale, m_pWindow->getPosition().d_y.d_scale);
    }
    else
    {
        CEGUI::Window*  pParent = m_pWindow->getParent();
        CEGUI::Vector2f absPos = CEGUI::CoordConverter::asAbsolute(m_pWindow->getPosition(), pParent ? pParent->getPixelSize() : CEGUI::Sizef(0.0f, 0.0f));
        return CVector2D(absPos.d_x, absPos.d_y);
    }
#else
    CEGUI::Point CEGUITemp;

    if (bRelative)
        CEGUITemp = m_pWindow->getPosition(CEGUI::Relative);
    else
        CEGUITemp = m_pWindow->getPosition(CEGUI::Absolute);

    return CVector2D(CEGUITemp.d_x, CEGUITemp.d_y);
#endif
}

void CGUIElement_Impl::GetPosition(CVector2D& vecPosition, bool bRelative)
{
#ifdef MTA_USE_CEGUI_NEXT
    vecPosition = GetPosition(bRelative);
#else
    CEGUI::MetricsMode type = CEGUI::Absolute;

    if (bRelative)
        type = CEGUI::Relative;

    CEGUI::Point Temp = m_pWindow->getPosition(type);

    vecPosition.fX = Temp.d_x;
    vecPosition.fY = Temp.d_y;
#endif
}

void CGUIElement_Impl::SetWidth(float fX, bool bRelative)
{
#ifdef MTA_USE_CEGUI_NEXT
    if (bRelative)
        m_pWindow->setWidth(CEGUI::UDim(fX, 0.0f));
    else
        m_pWindow->setWidth(CEGUI::UDim(0.0f, fX));
#else
    if (bRelative)
        m_pWindow->setWidth(CEGUI::Relative, fX);
    else
        m_pWindow->setWidth(CEGUI::Absolute, fX);
#endif
}

void CGUIElement_Impl::SetHeight(float fY, bool bRelative)
{
#ifdef MTA_USE_CEGUI_NEXT
    if (bRelative)
        m_pWindow->setHeight(CEGUI::UDim(fY, 0.0f));
    else
        m_pWindow->setHeight(CEGUI::UDim(0.0f, fY));
#else
    if (bRelative)
        m_pWindow->setHeight(CEGUI::Relative, fY);
    else
        m_pWindow->setHeight(CEGUI::Absolute, fY);
#endif
}

void CGUIElement_Impl::SetSize(const CVector2D& vecSize, bool bRelative)
{
#ifdef MTA_USE_CEGUI_NEXT
    if (bRelative)
        m_pWindow->setSize(CEGUI::USize(CEGUI::UDim(vecSize.fX, 0.0f), CEGUI::UDim(vecSize.fY, 0.0f)));
    else
        m_pWindow->setSize(CEGUI::USize(CEGUI::UDim(0.0f, vecSize.fX), CEGUI::UDim(0.0f, vecSize.fY)));
#else
    if (bRelative)
        m_pWindow->setSize(CEGUI::Relative, CEGUI::Size(vecSize.fX, vecSize.fY));
    else
        m_pWindow->setSize(CEGUI::Absolute, CEGUI::Size(vecSize.fX, vecSize.fY));
#endif

    CorrectEdges();
}

CVector2D CGUIElement_Impl::GetSize(bool bRelative)
{
#ifdef MTA_USE_CEGUI_NEXT
    if (bRelative)
    {
        CEGUI::Window* pParent = m_pWindow->getParent();
        if (pParent)
        {
            CEGUI::Sizef parentSize = pParent->getPixelSize();
            if (parentSize.d_width != 0.0f && parentSize.d_height != 0.0f)
            {
                CEGUI::Sizef pixelSize = m_pWindow->getPixelSize();
                return CVector2D(pixelSize.d_width / parentSize.d_width, pixelSize.d_height / parentSize.d_height);
            }
        }
        return CVector2D(m_pWindow->getSize().d_width.d_scale, m_pWindow->getSize().d_height.d_scale);
    }
    else
    {
        CEGUI::Sizef pixelSize = m_pWindow->getPixelSize();
        return CVector2D(pixelSize.d_width, pixelSize.d_height);
    }
#else
    CEGUI::Size TempSize;

    if (bRelative)
        TempSize = m_pWindow->getRelativeSize();
    else
        TempSize = m_pWindow->getAbsoluteSize();

    return CVector2D(TempSize.d_width, TempSize.d_height);
#endif
}

void CGUIElement_Impl::GetSize(CVector2D& vecSize, bool bRelative)
{
#ifdef MTA_USE_CEGUI_NEXT
    vecSize = GetSize(bRelative);
#else
    CEGUI::Size TempSize;

    if (bRelative)
        TempSize = m_pWindow->getRelativeSize();
    else
        TempSize = m_pWindow->getAbsoluteSize();

    vecSize.fX = TempSize.d_width;
    vecSize.fY = TempSize.d_height;
#endif
}

void CGUIElement_Impl::AutoSize(const char* Text, float fPaddingX, float fPaddingY)
{
    const CEGUI::Font* pFont = m_pWindow->getFont();
    if (!pFont)
        return;

#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setSize(CEGUI::USize(CEGUI::UDim(0.0f, pFont->getTextExtent(CGUI_Impl::GetUTFString(Text ? Text : GetText())) + fPaddingX),
                                    CEGUI::UDim(0.0f, pFont->getFontHeight() + fPaddingY)));
#else
    m_pWindow->setSize(CEGUI::Absolute,
                       CEGUI::Size(pFont->getTextExtent(CGUI_Impl::GetUTFString(Text ? Text : GetText())) + fPaddingX,
                                   pFont->getFontHeight() + fPaddingY));  // Add hack factor to height to allow for long characters such as 'g' or 'j'
#endif
}

void CGUIElement_Impl::SetMinimumSize(const CVector2D& vecSize)
{
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setMinSize(CEGUI::USize(CEGUI::UDim(0.0f, vecSize.fX), CEGUI::UDim(0.0f, vecSize.fY)));
#else
    m_pWindow->setMetricsMode(CEGUI::Absolute);
    m_pWindow->setMinimumSize(CEGUI::Size(vecSize.fX, vecSize.fY));
#endif
}

CVector2D CGUIElement_Impl::GetMinimumSize()
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::Window* pParent = m_pWindow->getParent();
    CEGUI::Sizef   minSize = CEGUI::CoordConverter::asAbsolute(m_pWindow->getMinSize(), pParent ? pParent->getPixelSize() : CEGUI::Sizef(0.0f, 0.0f));
    return CVector2D(minSize.d_width, minSize.d_height);
#else
    const CEGUI::Size& TempSize = m_pWindow->getMinimumSize();
    return CVector2D(TempSize.d_width, TempSize.d_height);
#endif
}

void CGUIElement_Impl::GetMinimumSize(CVector2D& vecSize)
{
#ifdef MTA_USE_CEGUI_NEXT
    vecSize = GetMinimumSize();
#else
    const CEGUI::Size& Temp = m_pWindow->getMinimumSize();
    vecSize.fX = Temp.d_width;
    vecSize.fY = Temp.d_height;
#endif
}

void CGUIElement_Impl::SetMaximumSize(const CVector2D& vecSize)
{
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setMaxSize(CEGUI::USize(CEGUI::UDim(0.0f, vecSize.fX), CEGUI::UDim(0.0f, vecSize.fY)));
#else
    m_pWindow->setMaximumSize(CEGUI::Size(vecSize.fX, vecSize.fY));
#endif
}

CVector2D CGUIElement_Impl::GetMaximumSize()
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::Window* pParent = m_pWindow->getParent();
    CEGUI::Sizef   maxSize = CEGUI::CoordConverter::asAbsolute(m_pWindow->getMaxSize(), pParent ? pParent->getPixelSize() : CEGUI::Sizef(0.0f, 0.0f));
    return CVector2D(maxSize.d_width, maxSize.d_height);
#else
    const CEGUI::Size& TempSize = m_pWindow->getMaximumSize();
    return CVector2D(TempSize.d_width, TempSize.d_height);
#endif
}

void CGUIElement_Impl::GetMaximumSize(CVector2D& vecSize)
{
#ifdef MTA_USE_CEGUI_NEXT
    vecSize = GetMaximumSize();
#else
    const CEGUI::Size& Temp = m_pWindow->getSize();
    vecSize.fX = Temp.d_width;
    vecSize.fY = Temp.d_height;
#endif
}

void CGUIElement_Impl::SetText(const char* szText)
{
    m_pWindow->setText(CGUI_Impl::GetUTFString(szText));
}

std::string CGUIElement_Impl::GetText()
{
#ifdef MTA_USE_CEGUI_NEXT
    return m_pWindow->getText().c_str();
#else
    return CGUI_Impl::GetUTFString(m_pWindow->getText().c_str()).c_str();
#endif
}

void CGUIElement_Impl::SetAlpha(float fAlpha)
{
    m_pWindow->setAlpha(fAlpha);
}

float CGUIElement_Impl::GetAlpha()
{
    return m_pWindow->getAlpha();
}

float CGUIElement_Impl::GetEffectiveAlpha()
{
    return m_pWindow->getEffectiveAlpha();
}

void CGUIElement_Impl::SetInheritsAlpha(bool bInheritsAlpha)
{
    m_pWindow->setInheritsAlpha(bInheritsAlpha);
}

bool CGUIElement_Impl::GetInheritsAlpha()
{
    return m_pWindow->inheritsAlpha();
}

void CGUIElement_Impl::Activate()
{
    m_pWindow->activate();
}

void CGUIElement_Impl::Deactivate()
{
    m_pWindow->deactivate();
}

bool CGUIElement_Impl::IsActive()
{
    return m_pWindow->isActive();
}

void CGUIElement_Impl::SetAlwaysOnTop(bool bAlwaysOnTop)
{
    m_pWindow->setAlwaysOnTop(bAlwaysOnTop);
}

bool CGUIElement_Impl::IsAlwaysOnTop()
{
    return m_pWindow->isAlwaysOnTop();
}

CRect2D CGUIElement_Impl::AbsoluteToRelative(const CRect2D& Rect)
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::Sizef pixelSize = m_pWindow->getPixelSize();
    if (pixelSize.d_width != 0.0f && pixelSize.d_height != 0.0f)
    {
        return CRect2D(Rect.fX1 / pixelSize.d_width, Rect.fY1 / pixelSize.d_height, Rect.fX2 / pixelSize.d_width, Rect.fY2 / pixelSize.d_height);
    }
    return Rect;
#else
    CEGUI::Rect TempRect = CEGUI::Rect(Rect.fX1, Rect.fY1, Rect.fX2, Rect.fY2);
    TempRect = m_pWindow->absoluteToRelative(TempRect);
    return CRect2D(TempRect.d_left, TempRect.d_top, TempRect.d_right, TempRect.d_bottom);
#endif
}

CVector2D CGUIElement_Impl::AbsoluteToRelative(const CVector2D& Vector)
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::Sizef pixelSize = m_pWindow->getPixelSize();
    if (pixelSize.d_width != 0.0f && pixelSize.d_height != 0.0f)
    {
        return CVector2D(Vector.fX / pixelSize.d_width, Vector.fY / pixelSize.d_height);
    }
    return Vector;
#else
    CEGUI::Size TempSize = CEGUI::Size(Vector.fX, Vector.fY);
    TempSize = m_pWindow->absoluteToRelative(TempSize);
    return CVector2D(TempSize.d_width, TempSize.d_height);
#endif
}

CRect2D CGUIElement_Impl::RelativeToAbsolute(const CRect2D& Rect)
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::Sizef pixelSize = m_pWindow->getPixelSize();
    return CRect2D(Rect.fX1 * pixelSize.d_width, Rect.fY1 * pixelSize.d_height, Rect.fX2 * pixelSize.d_width, Rect.fY2 * pixelSize.d_height);
#else
    CEGUI::Rect TempRect = CEGUI::Rect(Rect.fX1, Rect.fY1, Rect.fX2, Rect.fY2);
    TempRect = m_pWindow->relativeToAbsolute(TempRect);
    return CRect2D(TempRect.d_left, TempRect.d_top, TempRect.d_right, TempRect.d_bottom);
#endif
}

CVector2D CGUIElement_Impl::RelativeToAbsolute(const CVector2D& Vector)
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::Sizef pixelSize = m_pWindow->getPixelSize();
    return CVector2D(Vector.fX * pixelSize.d_width, Vector.fY * pixelSize.d_height);
#else
    CEGUI::Size TempSize = CEGUI::Size(Vector.fX, Vector.fY);
    TempSize = m_pWindow->relativeToAbsolute(TempSize);
    return CVector2D(TempSize.d_width, TempSize.d_height);
#endif
}

void CGUIElement_Impl::SetParent(CGUIElement* pParent)
{
    // Disable z-sorting if the label has a parent
    if (GetType() == CGUI_LABEL)
        m_pWindow->setZOrderingEnabled(pParent == NULL);

    if (pParent)
    {
        CGUIElement_Impl* pElement = dynamic_cast<CGUIElement_Impl*>(pParent);
        if (pElement)
        {
#ifdef MTA_USE_CEGUI_NEXT
            pElement->m_pWindow->addChild(m_pWindow);
#else
            pElement->m_pWindow->addChildWindow(m_pWindow);
#endif
        }
    }
    m_pParent = pParent;
}

CGUIElement* CGUIElement_Impl::GetParent()
{
    // Validate
    if (m_pParent && m_pWindow && !m_pWindow->getParent())
        return NULL;

    return m_pParent;
}

void CGUIElement_Impl::CorrectEdges()
{
#ifndef MTA_USE_CEGUI_NEXT
    CEGUI::Point currentPoint = m_pWindow->getPosition(CEGUI::Absolute);
    CEGUI::Size  currentSize = m_pWindow->getSize(CEGUI::Absolute);
    // Label turns out to be buggy
    if (m_pWindow->getType() == "CGUI/StaticText")
        return;

    if (m_pWindow->getParent()->getType() == "CGUI/FrameWindow")
    {
        CEGUI::Size parentSize = m_pWindow->getParent()->getSize(CEGUI::Absolute);
        if (currentPoint.d_x < CGUI_NODRAW_LEFT)
            currentPoint.d_x += CGUI_NODRAW_LEFT - currentPoint.d_x;
        if (currentPoint.d_y < CGUI_NODRAW_TOP)
            currentPoint.d_y += CGUI_NODRAW_TOP - currentPoint.d_x;
        if ((currentSize.d_height + currentPoint.d_y) > (parentSize.d_height - CGUI_NODRAW_BOTTOM))
            currentSize.d_height -= (currentSize.d_height + currentPoint.d_y) - (parentSize.d_height - CGUI_NODRAW_BOTTOM);
        if ((currentSize.d_width + currentPoint.d_x) > (parentSize.d_width - CGUI_NODRAW_RIGHT))
            currentSize.d_width -= (currentSize.d_width + currentPoint.d_x) - (parentSize.d_width - CGUI_NODRAW_RIGHT);
        m_pWindow->setPosition(CEGUI::Absolute, currentPoint);
        m_pWindow->setSize(CEGUI::Absolute, currentSize);
    }
#endif
}

bool CGUIElement_Impl::SetFont(const char* szFontName)
{
    if (szFontName != nullptr && *szFontName != '\0')
    {
#ifdef MTA_USE_CEGUI_NEXT
        if (!CEGUI::FontManager::getSingleton().isDefined(CEGUI::String(szFontName)))
            return false;
#else
        if (!CEGUI::FontManager::getSingleton().isFontPresent(CEGUI::String(szFontName)))
            return false;
#endif
    }

    try
    {
        m_pWindow->setFont(CEGUI::String(szFontName));
        return true;
    }
    catch (CEGUI::Exception e)
    {
        return false;
    }
}

std::string CGUIElement_Impl::GetFont()
{
    try
    {
        const CEGUI::Font* pFont = m_pWindow->getFont();
        if (pFont)
        {
            // Return the contname. std::string will copy it.
            CEGUI::String strFontName = pFont->getName();
            return strFontName.c_str();
        }
    }
    catch (CEGUI::Exception e)
    {
    }

    return "";
}

#ifdef MTA_USE_CEGUI_NEXT
static const char* TranslateLegacyPropertyName(const char* szProperty)
{
    if (stricmp(szProperty, "CaratIndex") == 0)
        return "CaretIndex";
    if (stricmp(szProperty, "RollUpEnabled") == 0)
        return "RollupEnabled";
    if (stricmp(szProperty, "UnifiedAreaRect") == 0 || stricmp(szProperty, "AbsoluteAreaRect") == 0 || stricmp(szProperty, "AbsoluteRect") == 0)
        return "Area";
    if (stricmp(szProperty, "UnifiedPosition") == 0 || stricmp(szProperty, "AbsolutePosition") == 0)
        return "Position";
    if (stricmp(szProperty, "UnifiedSize") == 0 || stricmp(szProperty, "AbsoluteSize") == 0)
        return "Size";
    if (stricmp(szProperty, "UnifiedXPosition") == 0 || stricmp(szProperty, "AbsoluteXPosition") == 0)
        return "XPosition";
    if (stricmp(szProperty, "UnifiedYPosition") == 0 || stricmp(szProperty, "AbsoluteYPosition") == 0)
        return "YPosition";
    if (stricmp(szProperty, "UnifiedWidth") == 0 || stricmp(szProperty, "AbsoluteWidth") == 0)
        return "Width";
    if (stricmp(szProperty, "UnifiedHeight") == 0 || stricmp(szProperty, "AbsoluteHeight") == 0)
        return "Height";
    return szProperty;
}

static std::string TranslateLegacyPropertyValue(const char* szValue)
{
    if (!szValue)
        return "";

    // Convert legacy "set:ImagesetName image:ImageName" to "ImagesetName/ImageName"
    const char* szSet = strstr(szValue, "set:");
    const char* szImg = strstr(szValue, "image:");
    if (szSet && szImg && szSet < szImg)
    {
        std::string strSet(szSet + 4, szImg - (szSet + 4));
        size_t      last = strSet.find_last_not_of(" \t\r\n");
        if (last != std::string::npos)
            strSet = strSet.substr(0, last + 1);

        std::string strImg(szImg + 6);
        size_t      first = strImg.find_first_not_of(" \t\r\n");
        if (first != std::string::npos)
            strImg = strImg.substr(first);
        last = strImg.find_last_not_of(" \t\r\n");
        if (last != std::string::npos)
            strImg = strImg.substr(0, last + 1);

        return strSet + "/" + strImg;
    }

    return szValue;
}
#endif

void CGUIElement_Impl::SetProperty(const char* szProperty, const char* szValue)
{
#ifdef MTA_USE_CEGUI_NEXT
    szProperty = TranslateLegacyPropertyName(szProperty);
    std::string strTranslatedValue = TranslateLegacyPropertyValue(szValue);
    szValue = strTranslatedValue.c_str();
#endif
    try
    {
        m_pWindow->setProperty(CGUI_Impl::GetUTFString(szProperty), CGUI_Impl::GetUTFString(szValue));
    }
    catch (const CEGUI::Exception&)
    {
    }
}

std::string CGUIElement_Impl::GetProperty(const char* szProperty)
{
#ifdef MTA_USE_CEGUI_NEXT
    szProperty = TranslateLegacyPropertyName(szProperty);
#endif
    CEGUI::String strValue;
    try
    {
        // Return the string. std::string will copy it
        strValue = CGUI_Impl::GetUTFString(m_pWindow->getProperty(CGUI_Impl::GetUTFString(szProperty)).c_str());
    }
    catch (const CEGUI::Exception&)
    {
    }

    return strValue.c_str();
}

void CGUIElement_Impl::FillProperties()
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::PropertySet::PropertyIterator itPropertySet = ((CEGUI::PropertySet*)m_pWindow)->getPropertyIterator();
#else
    CEGUI::Window::PropertyIterator itPropertySet = ((CEGUI::PropertySet*)m_pWindow)->getIterator();
#endif
    while (!itPropertySet.isAtEnd())
    {
        CEGUI::String strKey = itPropertySet.getCurrentKey();
        CEGUI::String strValue;
        try
        {
            strValue = m_pWindow->getProperty(strKey);
        }
        catch (const CEGUI::Exception&)
        {
            // CEGUI 0.8.7 has write-only properties (MultiColumnList's "ColumnHeader") that throw on read.
            // Letting it escape makes GetPropertiesBegin return a null iterator, so guiGetProperties on a
            // gridlist crashed the client. Skip it, like 0.4 which just returned nothing for it.
            itPropertySet++;
            continue;
        }

        CGUIProperty* pProperty = new CGUIProperty;
        pProperty->strKey = strKey.c_str();
        pProperty->strValue = strValue.c_str();

        m_Properties.push_back(pProperty);
        itPropertySet++;
    }
}

void CGUIElement_Impl::EmptyProperties()
{
    if (!m_Properties.empty())
    {
        CGUIPropertyIter iter = m_Properties.begin();
        CGUIPropertyIter iterEnd = m_Properties.end();
        for (; iter != iterEnd; iter++)
        {
            if (*iter)
            {
                delete (*iter);
            }
        }
    }
}

CGUIPropertyIter CGUIElement_Impl::GetPropertiesBegin()
{
    try
    {
        // Fill the properties list, if it's still empty (on first call)
        if (m_Properties.empty())
            FillProperties();

        // Return the list begin iterator
        return m_Properties.begin();
    }
    catch (CEGUI::Exception e)
    {
        // Dereferencing NULL here crashed the client; an empty range (begin == end) just yields no properties
        return m_Properties.end();
    }
}

CGUIPropertyIter CGUIElement_Impl::GetPropertiesEnd()
{
    try
    {
        // Fill the properties list, if it's still empty (on first call)
        if (m_Properties.empty())
            FillProperties();

        // Return the list begin iterator
        return m_Properties.end();
    }
    catch (CEGUI::Exception e)
    {
        // Dereferencing NULL here crashed the client; an empty range (begin == end) just yields no properties
        return m_Properties.end();
    }
}

void CGUIElement_Impl::SetMovedHandler(GUI_CALLBACK Callback)
{
    m_OnMoved = Callback;
}

void CGUIElement_Impl::SetSizedHandler(GUI_CALLBACK Callback)
{
    m_OnSized = Callback;
}

void CGUIElement_Impl::SetClickHandler(GUI_CALLBACK Callback)
{
    m_OnClick = Callback;
}

void CGUIElement_Impl::SetClickHandler(const GUI_CALLBACK_MOUSE& Callback)
{
    m_OnClickWithArgs = Callback;
}

void CGUIElement_Impl::SetDoubleClickHandler(GUI_CALLBACK Callback)
{
    m_OnDoubleClick = Callback;
}

void CGUIElement_Impl::SetMouseEnterHandler(GUI_CALLBACK Callback)
{
    m_OnMouseEnter = Callback;
}

void CGUIElement_Impl::SetMouseLeaveHandler(GUI_CALLBACK Callback)
{
    m_OnMouseLeave = Callback;
}

void CGUIElement_Impl::SetMouseButtonDownHandler(GUI_CALLBACK Callback)
{
    m_OnMouseDown = Callback;
}

void CGUIElement_Impl::SetActivateHandler(GUI_CALLBACK Callback)
{
    m_OnActivate = Callback;
}

void CGUIElement_Impl::SetDeactivateHandler(GUI_CALLBACK Callback)
{
    m_OnDeactivate = Callback;
}

void CGUIElement_Impl::SetKeyDownHandler(GUI_CALLBACK Callback)
{
    m_OnKeyDown = Callback;
}

void CGUIElement_Impl::SetEnterKeyHandler(GUI_CALLBACK Callback)
{
    m_OnEnter = Callback;
}

void CGUIElement_Impl::SetKeyDownHandler(const GUI_CALLBACK_KEY& Callback)
{
    m_OnKeyDownWithArgs = Callback;
}

void CGUIElement_Impl::AddEvents()
{
    // Note: Mouse Click, Double Click, Enter, Leave and ButtonDown are handled by global events in CGUI_Impl
    // Register our events
    m_pWindow->subscribeEvent(CEGUI::Window::EventMoved, CEGUI::Event::Subscriber(&CGUIElement_Impl::Event_OnMoved, this));
    m_pWindow->subscribeEvent(CEGUI::Window::EventSized, CEGUI::Event::Subscriber(&CGUIElement_Impl::Event_OnSized, this));
    m_pWindow->subscribeEvent(CEGUI::Window::EventActivated, CEGUI::Event::Subscriber(&CGUIElement_Impl::Event_OnActivated, this));
    m_pWindow->subscribeEvent(CEGUI::Window::EventDeactivated, CEGUI::Event::Subscriber(&CGUIElement_Impl::Event_OnDeactivated, this));
    m_pWindow->subscribeEvent(CEGUI::Window::EventKeyDown, CEGUI::Event::Subscriber(&CGUIElement_Impl::Event_OnKeyDown, this));
}

bool CGUIElement_Impl::Event_OnMoved(const CEGUI::EventArgs& e)
{
    if (m_OnMoved)
        m_OnMoved(reinterpret_cast<CGUIElement*>(this));
    return true;
}

bool CGUIElement_Impl::Event_OnSized(const CEGUI::EventArgs& e)
{
    if (m_OnSized)
        m_OnSized(reinterpret_cast<CGUIElement*>(this));
    return true;
}

bool CGUIElement_Impl::Event_OnClick(const CEGUI::EventArgs& eBase)
{
    const CEGUI::MouseEventArgs& e = reinterpret_cast<const CEGUI::MouseEventArgs&>(eBase);
    CGUIElement*                 pElement = reinterpret_cast<CGUIElement*>(this);

    if (m_OnClick)
        m_OnClick(pElement);

    if (m_OnClickWithArgs)
    {
        CGUIMouseEventArgs NewArgs;

        // copy the variables
        NewArgs.button = static_cast<CGUIMouse::MouseButton>(e.button);
        NewArgs.moveDelta = CVector2D(e.moveDelta.d_x, e.moveDelta.d_y);
        NewArgs.position = CGUIPosition(e.position.d_x, e.position.d_y);
        NewArgs.sysKeys = e.sysKeys;
        NewArgs.wheelChange = e.wheelChange;
        NewArgs.pWindow = pElement;

        m_OnClickWithArgs(NewArgs);
    }

    return true;
}

bool CGUIElement_Impl::Event_OnDoubleClick()
{
    if (m_OnDoubleClick)
        m_OnDoubleClick(reinterpret_cast<CGUIElement*>(this));
    return true;
}

bool CGUIElement_Impl::Event_OnMouseEnter()
{
    if (m_OnMouseEnter)
        m_OnMouseEnter(reinterpret_cast<CGUIElement*>(this));
    return true;
}

bool CGUIElement_Impl::Event_OnMouseLeave()
{
    if (m_OnMouseLeave)
        m_OnMouseLeave(reinterpret_cast<CGUIElement*>(this));
    return true;
}

bool CGUIElement_Impl::Event_OnMouseButtonDown()
{
    if (m_OnMouseDown)
        m_OnMouseDown(reinterpret_cast<CGUIElement*>(this));
    return true;
}

bool CGUIElement_Impl::Event_OnActivated(const CEGUI::EventArgs& e)
{
    if (m_OnActivate)
        m_OnActivate(reinterpret_cast<CGUIElement*>(this));
    return true;
}

bool CGUIElement_Impl::Event_OnDeactivated(const CEGUI::EventArgs& e)
{
    if (m_OnDeactivate)
        m_OnDeactivate(reinterpret_cast<CGUIElement*>(this));
    return true;
}

bool CGUIElement_Impl::Event_OnKeyDown(const CEGUI::EventArgs& e)
{
    const CEGUI::KeyEventArgs& Args = reinterpret_cast<const CEGUI::KeyEventArgs&>(e);
    CGUIElement*               pCGUIElement = reinterpret_cast<CGUIElement*>(this);

    if (m_OnKeyDown)
    {
        m_OnKeyDown(pCGUIElement);
    }

    if (m_OnKeyDownWithArgs)
    {
        CGUIKeyEventArgs NewArgs;

        // copy the variables
        NewArgs.codepoint = Args.codepoint;
        NewArgs.scancode = (CGUIKeys::Scan)Args.scancode;
        NewArgs.sysKeys = Args.sysKeys;

        // get the CGUIElement
        CGUIElement* pElement = reinterpret_cast<CGUIElement*>((Args.window)->getUserData());
        NewArgs.pWindow = pElement;

        m_OnKeyDownWithArgs(NewArgs);
    }

    if (m_OnEnter)
    {
        switch (Args.scancode)
        {
            // Return key
            case CEGUI::Key::NumpadEnter:
            case CEGUI::Key::Return:
            {
                // Fire the event
                m_OnEnter(pCGUIElement);
                break;
            }
        }
    }

    return false;
}

inline void CGUIElement_Impl::ForceRedraw()
{
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->invalidate();
#else
    m_pWindow->forceRedraw();
#endif
}

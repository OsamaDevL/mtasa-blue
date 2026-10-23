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
    // The unified values are written the same way in 0.4 and 0.8.7 ("{{s,o},{s,o}}"), so a rename is enough.
    // Absolute*, UnifiedX/YPosition and UnifiedWidth/Height have no 0.8.7 property at all and are handled
    // by GetLegacyGeometryProperty / SetLegacyGeometryProperty instead.
    if (stricmp(szProperty, "UnifiedAreaRect") == 0)
        return "Area";
    if (stricmp(szProperty, "UnifiedPosition") == 0)
        return "Position";
    if (stricmp(szProperty, "UnifiedSize") == 0)
        return "Size";
    if (stricmp(szProperty, "UnifiedMinSize") == 0)
        return "MinSize";
    if (stricmp(szProperty, "UnifiedMaxSize") == 0)
        return "MaxSize";
    // Plain renames between 0.4 and 0.8.7 (same meaning and value format)
    if (stricmp(szProperty, "RiseOnClick") == 0)
        return "RiseOnClickEnabled";
    if (stricmp(szProperty, "ZOrderChangeEnabled") == 0)
        return "ZOrderingEnabled";
    if (stricmp(szProperty, "Tooltip") == 0)
        return "TooltipText";
    if (stricmp(szProperty, "CustomTooltipType") == 0)
        return "TooltipType";
    if (stricmp(szProperty, "MouseButtonDownAutoRepeat") == 0)
        return "MouseAutoRepeatEnabled";
    if (stricmp(szProperty, "EditSelectionStart") == 0)
        return "SelectionStart";
    if (stricmp(szProperty, "EditSelectionLength") == 0)
        return "SelectionLength";
    if (stricmp(szProperty, "MaxEditTextLength") == 0)
        return "MaxTextLength";
    return szProperty;
}

// CEGUI 0.4 had a property for each axis (UnifiedXPosition, UnifiedWidth, ...) and pixel versions
// (AbsolutePosition "x:10 y:30", AbsoluteSize "w:.. h:..", AbsoluteRect "l:.. t:.. r:.. b:..", AbsoluteWidth "10").
// 0.8.7 only keeps Position/Size/Area, so scripts using the old names read "" and their writes were ignored.
// Mapping the Absolute* names onto the unified properties is not enough either: the pixel text doesn't parse as
// a unified value and would move or resize the element to zero. So read and write them through the Element API.
enum class ELegacyGeometry
{
    None,
    UnifiedX,
    UnifiedY,
    UnifiedWidth,
    UnifiedHeight,
    AbsoluteX,
    AbsoluteY,
    AbsoluteWidth,
    AbsoluteHeight,
    AbsolutePosition,
    AbsoluteSize,
    AbsoluteRect,
};

static ELegacyGeometry GetLegacyGeometry(const char* szProperty)
{
    static const struct
    {
        const char*     szName;
        ELegacyGeometry type;
    } names[] = {
        {"UnifiedXPosition", ELegacyGeometry::UnifiedX},         {"UnifiedYPosition", ELegacyGeometry::UnifiedY},
        {"UnifiedWidth", ELegacyGeometry::UnifiedWidth},         {"UnifiedHeight", ELegacyGeometry::UnifiedHeight},
        {"AbsoluteXPosition", ELegacyGeometry::AbsoluteX},       {"AbsoluteYPosition", ELegacyGeometry::AbsoluteY},
        {"AbsoluteWidth", ELegacyGeometry::AbsoluteWidth},       {"AbsoluteHeight", ELegacyGeometry::AbsoluteHeight},
        {"AbsolutePosition", ELegacyGeometry::AbsolutePosition}, {"AbsoluteSize", ELegacyGeometry::AbsoluteSize},
        {"AbsoluteRect", ELegacyGeometry::AbsoluteRect},         {"AbsoluteAreaRect", ELegacyGeometry::AbsoluteRect},
    };
    for (const auto& entry : names)
        if (stricmp(szProperty, entry.szName) == 0)
            return entry.type;
    return ELegacyGeometry::None;
}

static std::string FormatUDim(const CEGUI::UDim& value)
{
    // Same "{scale,offset}" text the 0.8.7 Position/Size properties produce
    return CEGUI::PropertyHelper<CEGUI::UDim>::toString(value).c_str();
}

static bool GetLegacyGeometryProperty(CEGUI::Window* pWindow, const char* szProperty, std::string& strOut)
{
    const ELegacyGeometry type = GetLegacyGeometry(szProperty);
    if (type == ELegacyGeometry::None)
        return false;

    // Pixel values are relative to the parent, like in 0.4, and use its "%f" text format
    const CEGUI::Sizef parentSize = pWindow->getParentPixelSize();
    const CEGUI::Sizef pixelSize = pWindow->getPixelSize();
    const float        x = CEGUI::CoordConverter::asAbsolute(pWindow->getXPosition(), parentSize.d_width);
    const float        y = CEGUI::CoordConverter::asAbsolute(pWindow->getYPosition(), parentSize.d_height);

    switch (type)
    {
        case ELegacyGeometry::UnifiedX:
            strOut = FormatUDim(pWindow->getXPosition());
            break;
        case ELegacyGeometry::UnifiedY:
            strOut = FormatUDim(pWindow->getYPosition());
            break;
        case ELegacyGeometry::UnifiedWidth:
            strOut = FormatUDim(pWindow->getWidth());
            break;
        case ELegacyGeometry::UnifiedHeight:
            strOut = FormatUDim(pWindow->getHeight());
            break;
        case ELegacyGeometry::AbsoluteX:
            strOut = SString("%f", x);
            break;
        case ELegacyGeometry::AbsoluteY:
            strOut = SString("%f", y);
            break;
        case ELegacyGeometry::AbsoluteWidth:
            strOut = SString("%f", pixelSize.d_width);
            break;
        case ELegacyGeometry::AbsoluteHeight:
            strOut = SString("%f", pixelSize.d_height);
            break;
        case ELegacyGeometry::AbsolutePosition:
            strOut = SString("x:%f y:%f", x, y);
            break;
        case ELegacyGeometry::AbsoluteSize:
            strOut = SString("w:%f h:%f", pixelSize.d_width, pixelSize.d_height);
            break;
        case ELegacyGeometry::AbsoluteRect:
            strOut = SString("l:%f t:%f r:%f b:%f", x, y, x + pixelSize.d_width, y + pixelSize.d_height);
            break;
        default:
            return false;
    }
    return true;
}

// Returns true when the name was one of ours (handled or rejected), false to fall through to setProperty.
// A value that doesn't parse changes nothing, as setting an unknown property did before.
static bool SetLegacyGeometryProperty(CEGUI::Window* pWindow, const char* szProperty, const char* szValue)
{
    const ELegacyGeometry type = GetLegacyGeometry(szProperty);
    if (type == ELegacyGeometry::None)
        return false;

    float a = 0, b = 0, c = 0, d = 0;
    switch (type)
    {
        case ELegacyGeometry::UnifiedX:
        case ELegacyGeometry::UnifiedY:
        case ELegacyGeometry::UnifiedWidth:
        case ELegacyGeometry::UnifiedHeight:
        {
            if (sscanf(szValue, " { %f , %f }", &a, &b) != 2)
                return true;
            const CEGUI::UDim value(a, b);
            if (type == ELegacyGeometry::UnifiedX)
                pWindow->setXPosition(value);
            else if (type == ELegacyGeometry::UnifiedY)
                pWindow->setYPosition(value);
            else if (type == ELegacyGeometry::UnifiedWidth)
                pWindow->setWidth(value);
            else
                pWindow->setHeight(value);
            return true;
        }
        case ELegacyGeometry::AbsoluteX:
        case ELegacyGeometry::AbsoluteY:
        case ELegacyGeometry::AbsoluteWidth:
        case ELegacyGeometry::AbsoluteHeight:
        {
            if (sscanf(szValue, " %f", &a) != 1)
                return true;
            const CEGUI::UDim value(0, a);
            if (type == ELegacyGeometry::AbsoluteX)
                pWindow->setXPosition(value);
            else if (type == ELegacyGeometry::AbsoluteY)
                pWindow->setYPosition(value);
            else if (type == ELegacyGeometry::AbsoluteWidth)
                pWindow->setWidth(value);
            else
                pWindow->setHeight(value);
            return true;
        }
        case ELegacyGeometry::AbsolutePosition:
            if (sscanf(szValue, " x:%f y:%f", &a, &b) == 2)
                pWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0, a), CEGUI::UDim(0, b)));
            return true;
        case ELegacyGeometry::AbsoluteSize:
            if (sscanf(szValue, " w:%f h:%f", &a, &b) == 2)
                pWindow->setSize(CEGUI::USize(CEGUI::UDim(0, a), CEGUI::UDim(0, b)));
            return true;
        case ELegacyGeometry::AbsoluteRect:
            if (sscanf(szValue, " l:%f t:%f r:%f b:%f", &a, &b, &c, &d) == 4)
                pWindow->setArea(CEGUI::URect(CEGUI::UDim(0, a), CEGUI::UDim(0, b), CEGUI::UDim(0, c), CEGUI::UDim(0, d)));
            return true;
        default:
            return true;
    }
}

// Scripts written for 0.4 compare read-back text, e.g. guiGetProperty(checkbox, "Selected") == "True".
// Give back the 0.4 text where 0.8.7 changed it. Numbers ("0.9" vs "0.900000") are left alone: tonumber reads both.
static CEGUI::String ToLegacyPropertyValue(CEGUI::Window* pWindow, const CEGUI::String& strName, const CEGUI::String& strValue)
{
    // 0.8.7 returns "" while the window uses the default font; 0.4 returned the name of the font in use
    if (strName == "Font")
    {
        if (strValue.empty())
        {
            const CEGUI::Font* pFont = pWindow->getFont();
            if (pFont)
                return pFont->getName();
        }
        return strValue;
    }

    // 0.4 wrote booleans as True/False, 0.8.7 as true/false
    if (strValue == "true" || strValue == "false")
    {
        const CEGUI::Property* pProperty = pWindow->getPropertyInstance(strName);
        if (pProperty && pProperty->getDataType() == "bool")
            return strValue == "true" ? "True" : "False";
        return strValue;
    }

    // Properties named "...Colour" (NormalTextColour, ActiveSelectionColour, ...) were a single "AARRGGBB" colour in
    // 0.4 but are corner sets in 0.8.7. "...Colours" (TextColours, ImageColours) were corner sets in 0.4 too.
    // Only collapse when all corners match; a real gradient can't be written as one colour.
    const size_t nameLength = strName.length();
    if (nameLength > 6 && strName.compare(nameLength - 6, 6, "Colour") == 0 && strValue.compare(0, 3, "tl:") == 0)
    {
        const CEGUI::ColourRect colours = CEGUI::PropertyHelper<CEGUI::ColourRect>::fromString(strValue);
        if (colours.isMonochromatic())
            return SString("%08X", colours.d_top_left.getARGB()).c_str();
    }

    return strValue;
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
    try
    {
        if (SetLegacyGeometryProperty(m_pWindow, szProperty, szValue ? szValue : ""))
            return;
    }
    catch (const CEGUI::Exception&)
    {
        return;
    }
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
    try
    {
        std::string strGeometry;
        if (GetLegacyGeometryProperty(m_pWindow, szProperty, strGeometry))
            return strGeometry;
    }
    catch (const CEGUI::Exception&)
    {
        return "";
    }
    szProperty = TranslateLegacyPropertyName(szProperty);
#endif
    CEGUI::String strValue;
    try
    {
        const CEGUI::String strName = CGUI_Impl::GetUTFString(szProperty);
#ifdef MTA_USE_CEGUI_NEXT
        const CEGUI::String strRaw = ToLegacyPropertyValue(m_pWindow, strName, m_pWindow->getProperty(strName));
#else
        const CEGUI::String strRaw = m_pWindow->getProperty(strName);
#endif
        // Return the string. std::string will copy it
        strValue = CGUI_Impl::GetUTFString(strRaw.c_str());
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
#ifdef MTA_USE_CEGUI_NEXT
            // Same 0.4-style text as guiGetProperty, so both functions agree
            strValue = ToLegacyPropertyValue(m_pWindow, strKey, strValue);
#endif
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

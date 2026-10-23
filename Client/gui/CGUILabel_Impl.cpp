/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        gui/CGUILabel_Impl.cpp
 *  PURPOSE:     Label widget class
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"

#ifdef MTA_USE_CEGUI_NEXT
    #include <CEGUI/PropertyHelper.h>
    #include <CEGUI/FreeTypeFont.h>
#endif

#define CGUILABEL_NAME "CGUI/StaticText"

CGUILabel_Impl::CGUILabel_Impl(CGUI_Impl* pGUI, CGUIElement* pParent, const char* szText)
{
    SetManager(pGUI);

    // Get an unique identifier for CEGUI (gah, there's gotta be an another way)
    char szUnique[CGUI_CHAR_SIZE];
    pGUI->GetUniqueName(szUnique);

    // Create the window and set default settings
    m_pWindow = pGUI->GetWindowManager()->createWindow(pGUI->ResolveWindowType(CGUILABEL_NAME, pParent), szUnique);
    m_pWindow->setDestroyedByParent(false);

    // Store the pointer to this CGUI element in the CEGUI element
    m_pWindow->setUserData(reinterpret_cast<void*>(this));

    AddEvents();

    // Do some hardcore disabling on the labels
    // m_pWindow->moveToBack ( );
    // m_pWindow->disable ( );

    // not sure what that was for, disabled
    // m_pWindow->setZOrderingEnabled ( false );
    // m_pWindow->setAlwaysOnTop ( true );

    SetFrameEnabled(false);
    SetHorizontalAlign(CGUI_ALIGN_LEFT);
    SetVerticalAlign(CGUI_ALIGN_TOP);
    SetText(szText);
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setProperty("BackgroundEnabled", "False");
#else
    reinterpret_cast<CEGUI::StaticText*>(m_pWindow)->setBackgroundEnabled(false);
#endif

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

CGUILabel_Impl::~CGUILabel_Impl()
{
    DestroyElement();
}

void CGUILabel_Impl::SetText(const char* Text)
{
    // Set the new text and size the text field after it
    m_pWindow->setText(CGUI_Impl::GetUTFString(Text));
}

void CGUILabel_Impl::SetVerticalAlign(CGUIVerticalAlign eAlign)
{
#ifdef MTA_USE_CEGUI_NEXT
    switch (eAlign)
    {
        case CGUI_ALIGN_TOP:
            m_pWindow->setProperty("VertFormatting", "TopAligned");
            break;
        case CGUI_ALIGN_VERTICALCENTER:
            m_pWindow->setProperty("VertFormatting", "CentreAligned");
            break;
        case CGUI_ALIGN_BOTTOM:
            m_pWindow->setProperty("VertFormatting", "BottomAligned");
            break;
    }
#else
    reinterpret_cast<CEGUI::StaticText*>(m_pWindow)->setVerticalFormatting(static_cast<CEGUI::StaticText::VertFormatting>(eAlign));
#endif
}

CGUIVerticalAlign CGUILabel_Impl::GetVerticalAlign()
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::String vert = m_pWindow->getProperty("VertFormatting");
    if (vert == "CentreAligned")
        return CGUI_ALIGN_VERTICALCENTER;
    if (vert == "BottomAligned")
        return CGUI_ALIGN_BOTTOM;
    return CGUI_ALIGN_TOP;
#else
    return static_cast<CGUIVerticalAlign>(reinterpret_cast<CEGUI::StaticText*>(m_pWindow)->getVerticalFormatting());
#endif
}

void CGUILabel_Impl::SetHorizontalAlign(CGUIHorizontalAlign eAlign)
{
#ifdef MTA_USE_CEGUI_NEXT
    switch (eAlign)
    {
        case CGUI_ALIGN_LEFT:
            m_pWindow->setProperty("HorzFormatting", "LeftAligned");
            break;
        case CGUI_ALIGN_RIGHT:
            m_pWindow->setProperty("HorzFormatting", "RightAligned");
            break;
        case CGUI_ALIGN_HORIZONTALCENTER:
            m_pWindow->setProperty("HorzFormatting", "CentreAligned");
            break;
        case CGUI_ALIGN_LEFT_WORDWRAP:
            m_pWindow->setProperty("HorzFormatting", "WordWrapLeftAligned");
            break;
        case CGUI_ALIGN_RIGHT_WORDWRAP:
            m_pWindow->setProperty("HorzFormatting", "WordWrapRightAligned");
            break;
        case CGUI_ALIGN_HORIZONTALCENTER_WORDWRAP:
            m_pWindow->setProperty("HorzFormatting", "WordWrapCentreAligned");
            break;
    }
#else
    reinterpret_cast<CEGUI::StaticText*>(m_pWindow)->setHorizontalFormatting(static_cast<CEGUI::StaticText::HorzFormatting>(eAlign));
#endif
}

CGUIHorizontalAlign CGUILabel_Impl::GetHorizontalAlign()
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::String horz = m_pWindow->getProperty("HorzFormatting");
    if (horz == "RightAligned")
        return CGUI_ALIGN_RIGHT;
    if (horz == "CentreAligned")
        return CGUI_ALIGN_HORIZONTALCENTER;
    if (horz == "WordWrapRightAligned")
        return CGUI_ALIGN_RIGHT_WORDWRAP;
    if (horz == "WordWrapCentreAligned")
        return CGUI_ALIGN_HORIZONTALCENTER_WORDWRAP;
    if (horz == "WordWrapLeftAligned")
        return CGUI_ALIGN_LEFT_WORDWRAP;
    return CGUI_ALIGN_LEFT;
#else
    return static_cast<CGUIHorizontalAlign>(reinterpret_cast<CEGUI::StaticText*>(m_pWindow)->getHorizontalFormatting());
#endif
}

void CGUILabel_Impl::SetTextColor(CGUIColor Color)
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::Colour col(Color.R / 255.0f, Color.G / 255.0f, Color.B / 255.0f, 1.0f);
    m_pWindow->setProperty("TextColours", CEGUI::PropertyHelper<CEGUI::ColourRect>::toString(CEGUI::ColourRect(col)));
#else
    reinterpret_cast<CEGUI::StaticText*>(m_pWindow)->setTextColours(CEGUI::colour(1.0f / 255.0f * Color.R, 1.0f / 255.0f * Color.G, 1.0f / 255.0f * Color.B));
#endif
}

void CGUILabel_Impl::SetTextColor(unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue)
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::Colour col(ucRed / 255.0f, ucGreen / 255.0f, ucBlue / 255.0f, 1.0f);
    m_pWindow->setProperty("TextColours", CEGUI::PropertyHelper<CEGUI::ColourRect>::toString(CEGUI::ColourRect(col)));
#else
    reinterpret_cast<CEGUI::StaticText*>(m_pWindow)->setTextColours(CEGUI::colour(1.0f / 255.0f * ucRed, 1.0f / 255.0f * ucGreen, 1.0f / 255.0f * ucBlue));
#endif
}

CGUIColor CGUILabel_Impl::GetTextColor()
{
    CGUIColor temp;
    GetTextColor(temp.R, temp.G, temp.B);
    return temp;
}

void CGUILabel_Impl::GetTextColor(unsigned char& ucRed, unsigned char& ucGreen, unsigned char& ucBlue)
{
#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::ColourRect cols = CEGUI::PropertyHelper<CEGUI::ColourRect>::fromString(m_pWindow->getProperty("TextColours"));
    CEGUI::Colour     r = cols.d_top_left;
    ucRed = static_cast<unsigned char>(r.getRed() * 255);
    ucGreen = static_cast<unsigned char>(r.getGreen() * 255);
    ucBlue = static_cast<unsigned char>(r.getBlue() * 255);
#else
    CEGUI::colour r = (reinterpret_cast<CEGUI::StaticText*>(m_pWindow)->getTextColours()).getColourAtPoint(0, 0);

    ucRed = (unsigned char)(r.getRed() * 255);
    ucGreen = (unsigned char)(r.getGreen() * 255);
    ucBlue = (unsigned char)(r.getBlue() * 255);
#endif
}

void CGUILabel_Impl::SetFrameEnabled(bool bFrameEnabled)
{
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setProperty("FrameEnabled", bFrameEnabled ? "True" : "False");
#else
    reinterpret_cast<CEGUI::StaticText*>(m_pWindow)->setFrameEnabled(bFrameEnabled);
#endif
}

bool CGUILabel_Impl::IsFrameEnabled()
{
#ifdef MTA_USE_CEGUI_NEXT
    return m_pWindow->getProperty("FrameEnabled") == "True";
#else
    return reinterpret_cast<CEGUI::StaticText*>(m_pWindow)->isFrameEnabled();
#endif
}

float CGUILabel_Impl::GetCharacterWidth(int iCharIndex)
{
    const CEGUI::Font* pFont = m_pWindow->getFont();
    if (pFont)
    {
        CEGUI::String text = m_pWindow->getText();
        if (iCharIndex >= 0 && static_cast<size_t>(iCharIndex) < text.length())
        {
            CEGUI::String charStr;
            charStr.append(1, text[iCharIndex]);
            return pFont->getTextExtent(charStr);
        }
    }
    return 0.0f;
}

float CGUILabel_Impl::GetFontHeight()
{
    const CEGUI::Font* pFont = m_pWindow->getFont();
    if (pFont)
    {
#ifdef MTA_USE_CEGUI_NEXT
        // Report the height the way 0.4 did (guiLabelGetFontHeight), see FreeTypeFont::getLegacyFontHeight
        if (const CEGUI::FreeTypeFont* pFreeTypeFont = dynamic_cast<const CEGUI::FreeTypeFont*>(pFont))
            return pFreeTypeFont->getLegacyFontHeight();
#endif
        return pFont->getFontHeight();
    }
    return 14.0f;
}

float CGUILabel_Impl::GetTextExtent()
{
    const CEGUI::Font* pFont = m_pWindow->getFont();
    if (pFont)
    {
        try
        {
            // Retrieve the longest line's extent
            std::stringstream ssText(m_pWindow->getText().c_str());
            std::string       sLineText;
            float             fMax = 0.0f, fLineExtent = 0.0f;

            while (std::getline(ssText, sLineText))
            {
                fLineExtent = pFont->getTextExtent(CGUI_Impl::GetUTFString(sLineText));
                if (fLineExtent > fMax)
                    fMax = fLineExtent;
            }
            return fMax;
        }
        catch (CEGUI::Exception e)
        {
        }
    }

    return 0.0f;
}

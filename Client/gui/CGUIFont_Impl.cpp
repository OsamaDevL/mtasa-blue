/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        gui/CGUIFont_Impl.cpp
 *  PURPOSE:     Font type class
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"

#ifdef MTA_USE_CEGUI_NEXT
    #include <CEGUI/FreeTypeFont.h>
#endif

CGUIFont_Impl::CGUIFont_Impl(CGUI_Impl* pGUI, const char* szFontName, const char* szFontFile, unsigned int uSize, unsigned int uFlags, bool bAutoScale)
{
    // Store the fontmanager and create a font with the given attributes
    m_pFontManager = pGUI->GetFontManager();
    m_pFont = NULL;

#ifdef MTA_USE_CEGUI_NEXT
    m_pFont = &m_pFontManager->createFreeTypeFont(szFontName, static_cast<float>(uSize), true, szFontFile, "",
                                                  bAutoScale ? CEGUI::ASM_Both : CEGUI::ASM_Disabled, CEGUI::Sizef(1024.0f, 768.0f));
#else
    while (!m_pFont)
    {
        try
        {
            m_pFont = m_pFontManager->createFont(szFontName, szFontFile, uSize, uFlags, bAutoScale, 1024, 768);
        }
        catch (CEGUI::RendererException)
        {
            // Reduce size until it can fit into a texture
            if (--uSize == 1)
                throw;
        }
    }

    // Define our glyphs
    m_pFont->setInitialFontGlyphs(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");

    // Set default attributes
    SetNativeResolution(1024, 768);
    SetAutoScalingEnabled(bAutoScale);
#endif
}

CGUIFont_Impl::~CGUIFont_Impl()
{
#ifdef MTA_USE_CEGUI_NEXT
    if (m_pFont)
        m_pFontManager->destroy(*m_pFont);
#else
    m_pFontManager->destroyFont(m_pFont);
#endif
}

void CGUIFont_Impl::SetAntiAliasingEnabled(bool bAntialiased)
{
#ifdef MTA_USE_CEGUI_NEXT
    if (m_pFont)
        static_cast<CEGUI::FreeTypeFont*>(m_pFont)->setAntiAliased(bAntialiased);
#else
    m_pFont->setAntiAliased(bAntialiased);
#endif
}

void CGUIFont_Impl::DrawTextString(const char* szText, CRect2D DrawArea, float fZ, CRect2D ClipRect, unsigned long ulFormat, unsigned long ulColor,
                                   float fScaleX, float fScaleY)
{
    if (!m_pFont || !szText)
        return;

#ifdef MTA_USE_CEGUI_NEXT
    CEGUI::Direct3D9Renderer* pRenderer = static_cast<CEGUI::Direct3D9Renderer*>(CEGUI::System::getSingleton().getRenderer());
    if (!pRenderer)
        return;

    CEGUI::GeometryBuffer& geomBuffer = pRenderer->createGeometryBuffer();
    CEGUI::Rectf           clip(ClipRect.fX1, ClipRect.fY1, ClipRect.fX2, ClipRect.fY2);
    CEGUI::ColourRect      colours = CEGUI::ColourRect(CEGUI::Colour(static_cast<CEGUI::argb_t>(ulColor)));

    m_pFont->drawText(geomBuffer, CGUI_Impl::GetUTFString(szText), CEGUI::Vector2f(DrawArea.fX1, DrawArea.fY1), &clip, colours, 0.0f, fScaleX, fScaleY);
    geomBuffer.draw();
    pRenderer->destroyGeometryBuffer(geomBuffer);
#else
    CEGUI::TextFormatting fmt;

    if (ulFormat == DT_CENTER)
        fmt = CEGUI::Centred;
    else if (ulFormat == DT_RIGHT)
        fmt = CEGUI::RightAligned;
    else
        fmt = CEGUI::LeftAligned;

    m_pFont->drawText(szText ? CGUI_Impl::GetUTFString(szText) : CEGUI::String(), CEGUI::Rect(DrawArea.fX1, DrawArea.fY1, DrawArea.fX2, DrawArea.fY2), fZ,
                      CEGUI::Rect(ClipRect.fX1, ClipRect.fY1, ClipRect.fX2, ClipRect.fY2), fmt, CEGUI::ColourRect(CEGUI::colour((CEGUI::argb_t)ulColor)),
                      fScaleX, fScaleY);
#endif
}

bool CGUIFont_Impl::IsAntiAliasingEnabled()
{
#ifdef MTA_USE_CEGUI_NEXT
    return m_pFont ? static_cast<CEGUI::FreeTypeFont*>(m_pFont)->isAntiAliased() : false;
#else
    return m_pFont->isAntiAliased();
#endif
}

void CGUIFont_Impl::SetAutoScalingEnabled(bool bAutoScaled)
{
#ifdef MTA_USE_CEGUI_NEXT
    if (m_pFont)
        m_pFont->setAutoScaled(bAutoScaled ? CEGUI::ASM_Both : CEGUI::ASM_Disabled);
#else
    m_pFont->setAutoScalingEnabled(bAutoScaled);
#endif
}

bool CGUIFont_Impl::IsAutoScalingEnabled()
{
#ifdef MTA_USE_CEGUI_NEXT
    return m_pFont ? (m_pFont->getAutoScaled() != CEGUI::ASM_Disabled) : false;
#else
    return m_pFont->isAutoScaled();
#endif
}

void CGUIFont_Impl::SetNativeResolution(int iX, int iY)
{
#ifdef MTA_USE_CEGUI_NEXT
    if (m_pFont)
        m_pFont->setNativeResolution(CEGUI::Sizef(static_cast<float>(iX), static_cast<float>(iY)));
#else
    m_pFont->setNativeResolution(CEGUI::Size(static_cast<float>(iX), static_cast<float>(iY)));
#endif
}

float CGUIFont_Impl::GetCharacterWidth(int iChar, float fScale)
{
    char szBuf[2];
    szBuf[0] = static_cast<char>(iChar);
    szBuf[1] = 0;

    return m_pFont->getTextExtent(szBuf, fScale);
}

float CGUIFont_Impl::GetFontHeight(float fScale)
{
#ifdef MTA_USE_CEGUI_NEXT
    // 0.8.7's getFontHeight is the design ascender - descender, smaller than what 0.4 reported. dxGetFontHeight and
    // friends must keep returning the 0.4 value, or script layouts built on it shift.
    const CEGUI::FreeTypeFont* pFreeTypeFont = dynamic_cast<const CEGUI::FreeTypeFont*>(m_pFont);
    float                      fHeight = pFreeTypeFont ? pFreeTypeFont->getLegacyFontHeight(fScale) : m_pFont->getFontHeight(fScale);
#else
    float fHeight = m_pFont->getFontHeight(fScale);  // average height.. not the maximum height for long characters such as 'g' or 'j'
#endif
    fHeight += 2.0f;  // so hack it

    return fHeight;
}

float CGUIFont_Impl::GetTextExtent(const char* szText, float fScale)
{
    return m_pFont->getTextExtent(CGUI_Impl::GetUTFString(szText), fScale);
}

CEGUI::Font* CGUIFont_Impl::GetFont()
{
    return m_pFont;
}

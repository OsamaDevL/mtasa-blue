/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        gui/CGUIScrollPane_Impl.cpp
 *  PURPOSE:     Scroll pane widget class
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"

#define CGUISCROLLPANE_NAME "CGUI/ScrollablePane"

CGUIScrollPane_Impl::CGUIScrollPane_Impl(CGUI_Impl* pGUI, CGUIElement* pParent)
{
    SetManager(pGUI);

    // Initialize
    m_pGUI = pGUI;

    // Get an unique identifier for CEGUI (gah, there's gotta be an another way)
    char szUnique[CGUI_CHAR_SIZE];
    pGUI->GetUniqueName(szUnique);

    // Create the window and set default settings
    m_pWindow = pGUI->GetWindowManager()->createWindow(pGUI->ResolveWindowType(CGUISCROLLPANE_NAME, pParent), szUnique);
    m_pWindow->setDestroyedByParent(false);
#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setArea(CEGUI::URect(CEGUI::UDim(0.0f, 0.0f), CEGUI::UDim(0.0f, 0.0f), CEGUI::UDim(0.9f, 0.0f), CEGUI::UDim(0.9f, 0.0f)));
#else
    m_pWindow->setRect(CEGUI::Relative, CEGUI::Rect(0.9f, 0.9f, 0.9f, 0.9f));
#endif

    // Store the pointer to this CGUI element in the CEGUI element
    m_pWindow->setUserData(reinterpret_cast<void*>(this));
    AddEvents();

    // If a parent is specified, add it to it's children list, if not, add it as a child to the m_pManager
    if (pParent)
    {
        SetParent(pParent);
    }
    else
    {
        pGUI->AddChild(this);
        SetParent(NULL);
    }
    SetHorizontalScrollStepSize(100.0f);
    SetVerticalScrollStepSize(100.0f);
}

CGUIScrollPane_Impl::~CGUIScrollPane_Impl()
{
    DestroyElement();
}

void CGUIScrollPane_Impl::SetHorizontalScrollBar(bool bEnabled)
{
    m_bHorzScrollbarEnabled = bEnabled;
#ifdef MTA_USE_CEGUI_NEXT
    auto* pPane = reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow);
    pPane->setShowHorzScrollbar(bEnabled);
    pPane->setEnableHorzScroll(bEnabled);
    if (!bEnabled && pPane->isChild(CEGUI::ScrollablePane::HorzScrollbarName))
    {
        pPane->getHorzScrollbar()->setVisible(false);
    }
#else
    reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow)->setShowHorzScrollbar(bEnabled);
#endif
}

void CGUIScrollPane_Impl::SetVerticalScrollBar(bool bEnabled)
{
    m_bVertScrollbarEnabled = bEnabled;
#ifdef MTA_USE_CEGUI_NEXT
    auto* pPane = reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow);
    pPane->setShowVertScrollbar(bEnabled);
    pPane->setEnableVertScroll(bEnabled);
    if (!bEnabled && pPane->isChild(CEGUI::ScrollablePane::VertScrollbarName))
    {
        pPane->getVertScrollbar()->setVisible(false);
    }
#else
    reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow)->setShowVertScrollbar(bEnabled);
#endif
}

void CGUIScrollPane_Impl::SetHorizontalScrollPosition(float fPosition)
{
    try
    {
        CEGUI::ScrollablePane* pScrollPane = reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow);
        float                  fFullWidth = pScrollPane->getContentPaneArea().getWidth();
#ifdef MTA_USE_CEGUI_NEXT
        float fViewWidth = pScrollPane->getPixelSize().d_width;
#else
        float fViewWidth = pScrollPane->getAbsoluteWidth();
#endif

        pScrollPane->setHorizontalScrollPosition(fPosition * ((fFullWidth - fViewWidth) / fFullWidth));
    }
    catch (CEGUI::Exception)
    {
    }
}

void CGUIScrollPane_Impl::SetVerticalScrollPosition(float fPosition)
{
    try
    {
        CEGUI::ScrollablePane* pScrollPane = reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow);
        float                  fFullHeight = pScrollPane->getContentPaneArea().getHeight();
#ifdef MTA_USE_CEGUI_NEXT
        float fViewHeight = pScrollPane->getPixelSize().d_height;
#else
        float fViewHeight = pScrollPane->getAbsoluteHeight();
#endif

        pScrollPane->setVerticalScrollPosition(fPosition * ((fFullHeight - fViewHeight) / fFullHeight));
    }
    catch (CEGUI::Exception)
    {
    }
}

float CGUIScrollPane_Impl::GetHorizontalScrollPosition()
{
    try
    {
        CEGUI::ScrollablePane* pScrollPane = reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow);

        float fFullWidth = pScrollPane->getContentPaneArea().getWidth();
#ifdef MTA_USE_CEGUI_NEXT
        float fViewWidth = pScrollPane->getPixelSize().d_width;
#else
        float fViewWidth = pScrollPane->getAbsoluteWidth();
#endif

        // Prevent division by zero
        if (fFullWidth == fViewWidth)
            return 0.0f;

        return (pScrollPane->getHorizontalScrollPosition() / ((fFullWidth - fViewWidth) / fFullWidth));
    }
    catch (CEGUI::Exception)
    {
        return 0;
    }
}

float CGUIScrollPane_Impl::GetVerticalScrollPosition()
{
    try
    {
        CEGUI::ScrollablePane* pScrollPane = reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow);

        float fFullHeight = pScrollPane->getContentPaneArea().getHeight();
#ifdef MTA_USE_CEGUI_NEXT
        float fViewHeight = pScrollPane->getPixelSize().d_height;
#else
        float fViewHeight = pScrollPane->getAbsoluteHeight();
#endif

        // Prevent division by zero
        if (fFullHeight == fViewHeight)
            return 0.0f;

        return (pScrollPane->getVerticalScrollPosition() / ((fFullHeight - fViewHeight) / fFullHeight));
    }
    catch (CEGUI::Exception)
    {
        return 0;
    }
}

void CGUIScrollPane_Impl::SetHorizontalScrollStepSize(float fPosition)
{
    try
    {
        reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow)->setHorizontalStepSize(fPosition);
    }
    catch (CEGUI::Exception)
    {
    }
}

void CGUIScrollPane_Impl::SetVerticalScrollStepSize(float fPosition)
{
    try
    {
        reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow)->setVerticalStepSize(fPosition);
    }
    catch (CEGUI::Exception)
    {
    }
}

float CGUIScrollPane_Impl::GetHorizontalScrollStepSize()
{
    try
    {
        return reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow)->getHorizontalStepSize();
    }
    catch (CEGUI::Exception)
    {
        return 0;
    }
}

float CGUIScrollPane_Impl::GetVerticalScrollStepSize()
{
    try
    {
        return reinterpret_cast<CEGUI::ScrollablePane*>(m_pWindow)->getVerticalStepSize();
    }
    catch (CEGUI::Exception)
    {
        return 0;
    }
}

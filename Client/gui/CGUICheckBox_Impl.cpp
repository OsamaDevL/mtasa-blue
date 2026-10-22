/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        gui/CGUICheckBox_Impl.cpp
 *  PURPOSE:     Checkbox widget class
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"

#define CGUICHECKBOX_NAME "CGUI/Checkbox"

CGUICheckBox_Impl::CGUICheckBox_Impl(CGUI_Impl* pGUI, CGUIElement* pParent, const char* szCaption, bool bChecked)
{
    SetManager(pGUI);

    // Get an unique identifier for CEGUI (gah, there's gotta be an another way)
    char szUnique[CGUI_CHAR_SIZE];
    pGUI->GetUniqueName(szUnique);

    // Create the window and set default settings
    m_pWindow = pGUI->GetWindowManager()->createWindow(pGUI->ResolveWindowType(CGUICHECKBOX_NAME, pParent), szUnique);
    m_pWindow->setDestroyedByParent(false);

    m_pWindow->setText(CGUI_Impl::GetUTFString(szCaption));

#ifdef MTA_USE_CEGUI_NEXT
    m_pWindow->setSize(CEGUI::USize(CEGUI::UDim(0.0f, 128.0f), CEGUI::UDim(0.0f, 16.0f)));
#else
    m_pWindow->setSize(CEGUI::Absolute, CEGUI::Size(128.0f, 16.0f));
#endif
    m_pWindow->setVisible(true);

    // Store the pointer to this CGUI element in the CEGUI element
    m_pWindow->setUserData(reinterpret_cast<void*>(this));

    // Register our events
    AddEvents();

    // Set selected state
    SetSelected(bChecked);

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

CGUICheckBox_Impl::~CGUICheckBox_Impl()
{
    DestroyElement();
}

// MTA stores the inverse of the checked state in CEGUI: in every skin the "unselected" image
// (CheckboxNormal) is the one with the tick. The 0.8.7 skins keep those images, so keep the inversion
// there too, otherwise every checkbox shows the opposite of its value.
void CGUICheckBox_Impl::SetSelected(bool bChecked)
{
#ifdef MTA_USE_CEGUI_NEXT
    reinterpret_cast<CEGUI::ToggleButton*>(m_pWindow)->setSelected(!bChecked);
#else
    reinterpret_cast<CEGUI::Checkbox*>(m_pWindow)->setSelected(!bChecked);
#endif
}

bool CGUICheckBox_Impl::GetSelected()
{
#ifdef MTA_USE_CEGUI_NEXT
    return !(reinterpret_cast<CEGUI::ToggleButton*>(m_pWindow)->isSelected());
#else
    return !(reinterpret_cast<CEGUI::Checkbox*>(m_pWindow)->isSelected());
#endif
}

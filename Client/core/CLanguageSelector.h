/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        core/CLanguageSelector.h
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

struct CLangListItem
{
    CLangListItem() : fFocusEffectPos(0), fFocusEffectTarget(0), pContainerPane(nullptr), pIcon(nullptr), pLabel(nullptr) {}
    float            fFocusEffectPos;
    float            fFocusEffectTarget;
    SString          strLocale;
    CGUIElement*     pContainerPane;
    CGUIStaticImage* pIcon;
    CVector2D        vecIconInitialPos;
    CVector2D        vecIconInitialSize;
    CGUILabel*       pLabel;
    CVector2D        vecLabelInitialPos;
    CVector2D        vecLabelInitialSize;
};

class CLanguageSelector
{
public:
    ZERO_ON_NEW
    // By default the button sits in the bottom-right corner and the list opens upwards. With pvecButtonPos
    // the button goes there instead and the list opens downwards (used by the modern main menu).
    CLanguageSelector(CGUIElement* pMainMenuCanvas, const CVector2D* pvecButtonPos = nullptr);
    ~CLanguageSelector();
    void DoPulse();

protected:
    void           CreateGUI(CGUIElement* pMainMenuCanvas, const CVector2D* pvecButtonPos);
    CLangListItem  CreateGUILangItem(CGUIElement* pGUIParent, const CVector2D& vecPanePosition, const SString& strLocale = "");
    void           SetLanguageListVisible(bool bVisible);
    CLangListItem* GetListItemByIndex(uint uiIndex);
    bool           OnButtonEnter(CGUIElement* pElement);
    bool           OnButtonLeave(CGUIElement* pElement);
    bool           OnButtonClick(CGUIElement* pElement);
    bool           OnListItemEnter(CGUIElement* pElement);
    bool           OnListItemLeave(CGUIElement* pElement);
    bool           OnListItemClick(CGUIElement* pElement);
    bool           OnListDeactivate(CGUIElement* pElement);

    CGUIScrollPane*            m_pButtonWindow;
    CGUIWindow*                m_pListWindow;
    CLangListItem              m_ButtonItem;
    std::vector<CLangListItem> m_ListItems;
    CTickCount                 m_LastTickCount;
    bool                       m_bMouseOverButton;
    bool                       m_bListWasVisible;
};

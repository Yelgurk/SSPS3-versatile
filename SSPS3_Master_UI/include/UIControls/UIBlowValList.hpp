#ifndef UIBlowValList_hpp
#define UIBlowValList_hpp

#include "../UIElement.hpp"

class UIBlowValList : public UIElement
{
public:
    UIBlowValList(UIElement * parent_navi, vector<KeyModel> key_press_actions)
    : UIElement
    {
        { EquipmentType::All },
        key_press_actions,
        true,
        false,
        false,
        PlaceControlIn::Control,
        parent_navi->get_screen(),
        parent_navi,
        { StyleActivator::Unscrollable, StyleActivator::Rectangle, StyleActivator::Focus }
    }
    {}
};

#endif
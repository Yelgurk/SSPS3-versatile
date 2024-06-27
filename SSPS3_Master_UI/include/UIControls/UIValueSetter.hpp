#ifndef UIObject_hpp
#define UIObject_hpp

#include "../UIElement.hpp"

class UIObject : public UIElement
{
public:
    UIObject(
        vector<EquipmentType> relates_to,
        vector<KeyModel> key_press_actions,
        UIElement * parent_navi,
        lv_img_dsc_t * header_icon,
        string header_text,
        uint32_t width,
        uint32_t height,
        lv_align_t align,
        int32_t offset_x = 0,
        int32_t offset_y = 0
    ) : UIElement {
        relates_to,
        key_press_actions,
        true,
        true,
        false,
        PlaceControlIn::Control,
        parent_navi->get_screen(),
        parent_navi,
        { StyleActivator::Focus, StyleActivator::Select, StyleActivator::Unscrollable, StyleActivator::Shadow }
    }
    {
        lv_obj_set_width(get_container(), width);
        lv_obj_set_height(get_container(), height);
        lv_obj_align(get_container(), align, offset_x, offset_y);
        lv_obj_set_style_bg_color(get_container(), COLOR_MEDIUM_GREY, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t * lv_setter_icon_up = lv_img_create(get_container());
        //////////////////////////lv_img_set_src(lv_setter_icon_up, &ui_img_sort_up_48_png);
        lv_obj_set_width(lv_setter_icon_up, LV_SIZE_CONTENT);   /// 24
        lv_obj_set_height(lv_setter_icon_up, LV_SIZE_CONTENT);    /// 24
        lv_obj_set_x(lv_setter_icon_up, 0);
        lv_obj_set_y(lv_setter_icon_up, 18);
        lv_obj_set_align(lv_setter_icon_up, LV_ALIGN_TOP_MID);
        lv_obj_add_flag(lv_setter_icon_up, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(lv_setter_icon_up, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_img_set_zoom(lv_setter_icon_up, 192);

        lv_obj_t * lv_setter_icon_down = lv_img_create(get_container());
        //////////////////////////////lv_img_set_src(lv_setter_icon_down, &ui_img_sort_down_48_png);
        lv_obj_set_width(lv_setter_icon_down, LV_SIZE_CONTENT);   /// 24
        lv_obj_set_height(lv_setter_icon_down, LV_SIZE_CONTENT);    /// 24
        lv_obj_set_x(lv_setter_icon_down, 0);
        lv_obj_set_y(lv_setter_icon_down, 10);
        lv_obj_set_align(lv_setter_icon_down, LV_ALIGN_BOTTOM_MID);
        lv_obj_add_flag(lv_setter_icon_down, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(lv_setter_icon_down, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_img_set_zoom(lv_setter_icon_down, 192);

        lv_obj_t * lv_setter_header_icon = lv_img_create(get_container());
        lv_obj_set_width(lv_setter_header_icon, 0);   /// 48
        lv_obj_set_height(lv_setter_header_icon, 0);    /// 48
        lv_obj_set_x(lv_setter_header_icon, 0);
        lv_obj_set_y(lv_setter_header_icon, -8);
        lv_obj_set_align(lv_setter_header_icon, LV_ALIGN_TOP_MID);
        lv_obj_add_flag(lv_setter_header_icon, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(lv_setter_header_icon, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_img_set_zoom(lv_setter_header_icon, 0);

        lv_obj_t * lv_setter_header_text = lv_label_create(get_container());
        lv_obj_set_width(lv_setter_header_text, 0);   /// 1
        lv_obj_set_height(lv_setter_header_text, 0);    /// 1
        lv_obj_set_x(lv_setter_header_text, 0);
        lv_obj_set_y(lv_setter_header_text, -8);
        lv_obj_set_align(lv_setter_header_text, LV_ALIGN_TOP_MID);
        ///////////////////////////lv_obj_set_style_text_font(lv_setter_header_text, &MontserratInt20, LV_PART_MAIN | LV_STATE_DEFAULT);

        if (header_icon != nullptr)
        {
            ////////////////////////////////lv_img_set_src(lv_setter_header_icon, &ui_img_fan_png);
            lv_obj_set_width(lv_setter_header_icon, LV_SIZE_CONTENT);   /// 48
            lv_obj_set_height(lv_setter_header_icon, LV_SIZE_CONTENT);    /// 48
            lv_img_set_zoom(lv_setter_header_icon, 160);
        }
        else
        {
            lv_label_set_text(lv_setter_header_text, header_text.c_str());
            lv_obj_set_width(lv_setter_header_text, LV_SIZE_CONTENT);   /// 1
            lv_obj_set_height(lv_setter_header_text, LV_SIZE_CONTENT);    /// 1
        }

        lv_obj_t * lv_setter_value = lv_label_create(get_container());
        lv_obj_set_width(lv_setter_value, LV_SIZE_CONTENT);   /// 1
        lv_obj_set_height(lv_setter_value, LV_SIZE_CONTENT);    /// 1
        lv_obj_set_x(lv_setter_value, 0);
        lv_obj_set_y(lv_setter_value, 14);
        lv_obj_set_align(lv_setter_value, LV_ALIGN_CENTER);
        lv_label_set_text(lv_setter_value, "25");
        ////////////////////////lv_obj_set_style_text_font(lv_setter_value, &MontserratInt20, LV_PART_MAIN | LV_STATE_DEFAULT);

        remember_child_element("[header]", lv_setter_header_text);
        remember_child_element("[value]", lv_setter_value);
    }
};

#endif
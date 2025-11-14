//
// Created by RM UI Designer
//

#include "ui_default_Ungroup_4.h"

#define FRAME_ID 0
#define GROUP_ID 0
#define START_ID 4
#define OBJ_NUM 7
#define FRAME_OBJ_NUM 7

CAT(ui_, CAT(FRAME_OBJ_NUM, _frame_t)) ui_default_Ungroup_4;
ui_interface_round_t *ui_default_Ungroup_NewRound = (ui_interface_round_t *)&(ui_default_Ungroup_4.data[0]);
ui_interface_rect_t *ui_default_Ungroup_NewRect = (ui_interface_rect_t *)&(ui_default_Ungroup_4.data[1]);
ui_interface_line_t *ui_default_Ungroup_NewLine = (ui_interface_line_t *)&(ui_default_Ungroup_4.data[2]);
ui_interface_ellipse_t *ui_default_Ungroup_NewEllipse = (ui_interface_ellipse_t *)&(ui_default_Ungroup_4.data[3]);
ui_interface_arc_t *ui_default_Ungroup_NewArc = (ui_interface_arc_t *)&(ui_default_Ungroup_4.data[4]);
ui_interface_number_t *ui_default_Ungroup_NewNumber = (ui_interface_number_t *)&(ui_default_Ungroup_4.data[5]);
ui_interface_number_t *ui_default_Ungroup_NewFloat = (ui_interface_number_t *)&(ui_default_Ungroup_4.data[6]);

void _ui_init_default_Ungroup_4() {
    for (int i = 0; i < OBJ_NUM; i++) {
        ui_default_Ungroup_4.data[i].figure_name[0] = FRAME_ID;
        ui_default_Ungroup_4.data[i].figure_name[1] = GROUP_ID;
        ui_default_Ungroup_4.data[i].figure_name[2] = i + START_ID;
        ui_default_Ungroup_4.data[i].operate_tpyel = 1;
    }
    for (int i = OBJ_NUM; i < FRAME_OBJ_NUM; i++) {
        ui_default_Ungroup_4.data[i].operate_tpyel = 0;
    }

    ui_default_Ungroup_NewRound->figure_tpye = 2;
    ui_default_Ungroup_NewRound->layer = 0;
    ui_default_Ungroup_NewRound->r = 173;
    ui_default_Ungroup_NewRound->start_x = 679;
    ui_default_Ungroup_NewRound->start_y = 606;
    ui_default_Ungroup_NewRound->color = 0;
    ui_default_Ungroup_NewRound->width = 1;

    ui_default_Ungroup_NewRect->figure_tpye = 1;
    ui_default_Ungroup_NewRect->layer = 0;
    ui_default_Ungroup_NewRect->start_x = 381;
    ui_default_Ungroup_NewRect->start_y = 435;
    ui_default_Ungroup_NewRect->color = 0;
    ui_default_Ungroup_NewRect->width = 1;
    ui_default_Ungroup_NewRect->end_x = 565;
    ui_default_Ungroup_NewRect->end_y = 619;

    ui_default_Ungroup_NewLine->figure_tpye = 0;
    ui_default_Ungroup_NewLine->layer = 0;
    ui_default_Ungroup_NewLine->start_x = 802;
    ui_default_Ungroup_NewLine->start_y = 278;
    ui_default_Ungroup_NewLine->end_x = 1279;
    ui_default_Ungroup_NewLine->end_y = 755;
    ui_default_Ungroup_NewLine->color = 0;
    ui_default_Ungroup_NewLine->width = 1;

    ui_default_Ungroup_NewEllipse->figure_tpye = 3;
    ui_default_Ungroup_NewEllipse->layer = 0;
    ui_default_Ungroup_NewEllipse->rx = 174;
    ui_default_Ungroup_NewEllipse->ry = 174;
    ui_default_Ungroup_NewEllipse->start_x = 761;
    ui_default_Ungroup_NewEllipse->start_y = 559;
    ui_default_Ungroup_NewEllipse->color = 0;
    ui_default_Ungroup_NewEllipse->width = 1;

    ui_default_Ungroup_NewArc->figure_tpye = 4;
    ui_default_Ungroup_NewArc->layer = 0;
    ui_default_Ungroup_NewArc->rx = 145;
    ui_default_Ungroup_NewArc->ry = 145;
    ui_default_Ungroup_NewArc->start_x = 393;
    ui_default_Ungroup_NewArc->start_y = 938;
    ui_default_Ungroup_NewArc->color = 0;
    ui_default_Ungroup_NewArc->width = 1;
    ui_default_Ungroup_NewArc->start_angle = 0;
    ui_default_Ungroup_NewArc->end_angle = 90;

    ui_default_Ungroup_NewNumber->figure_tpye = 6;
    ui_default_Ungroup_NewNumber->layer = 0;
    ui_default_Ungroup_NewNumber->font_size = 20;
    ui_default_Ungroup_NewNumber->start_x = 127;
    ui_default_Ungroup_NewNumber->start_y = 808;
    ui_default_Ungroup_NewNumber->color = 0;
    ui_default_Ungroup_NewNumber->number = 12345;
    ui_default_Ungroup_NewNumber->width = 2;

    ui_default_Ungroup_NewFloat->figure_tpye = 5;
    ui_default_Ungroup_NewFloat->layer = 0;
    ui_default_Ungroup_NewFloat->font_size = 20;
    ui_default_Ungroup_NewFloat->start_x = 114;
    ui_default_Ungroup_NewFloat->start_y = 748;
    ui_default_Ungroup_NewFloat->color = 0;
    ui_default_Ungroup_NewFloat->number = 114514;
    ui_default_Ungroup_NewFloat->width = 2;


    CAT(ui_proc_, CAT(FRAME_OBJ_NUM, _frame))(&ui_default_Ungroup_4);
    SEND_MESSAGE((uint8_t *) &ui_default_Ungroup_4, sizeof(ui_default_Ungroup_4));
}

void _ui_update_default_Ungroup_4() {
    for (int i = 0; i < OBJ_NUM; i++) {
        ui_default_Ungroup_4.data[i].operate_tpyel = 2;
    }

    CAT(ui_proc_, CAT(FRAME_OBJ_NUM, _frame))(&ui_default_Ungroup_4);
    SEND_MESSAGE((uint8_t *) &ui_default_Ungroup_4, sizeof(ui_default_Ungroup_4));
}

void _ui_remove_default_Ungroup_4() {
    for (int i = 0; i < OBJ_NUM; i++) {
        ui_default_Ungroup_4.data[i].operate_tpyel = 3;
    }

    CAT(ui_proc_, CAT(FRAME_OBJ_NUM, _frame))(&ui_default_Ungroup_4);
    SEND_MESSAGE((uint8_t *) &ui_default_Ungroup_4, sizeof(ui_default_Ungroup_4));
}

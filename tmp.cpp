if (menu_slots[i].isEditMode) {
    static uint8_t isCurrentEditSlot = 0;

    EditBlockParam(i);

    if (isCurrentEditSlot != i) {
        isCurrentEditSlot = i;

        for (size_t j = 0; j < NUM_MENU_SLOTS; j++) {
            if (j != i) {
                menu_slots[j].isEditMode = false;
                InitOneParamBlock(j, *allParams[menu_slots[j].assignedParam].target_param, 
                    allParams[menu_slots[j].assignedParam].label, WHITE, BLACK);
            }
        }
    }
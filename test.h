
struct MenuParameter{
    const char* name;
    float* value;
};

struct Slot {
    MenuParameter* menu_param;
};
struct Page {
    MenuParameter parameters[NUM_PARAMS_ON_PAGE];
};




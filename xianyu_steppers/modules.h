#include "motion.h"

#define UNKNOWN -2

#define CLAW_LAYERS 2 //(equals claw_float height)

//manages module lifecycle
//ensures safety
//exposes function for action sequences


typedef struct computed_states_t
{
    //claw machanics
    char claw_engaged; //0 no, 1 yes, -2 = invalid
    char claw_cargo_touched; //Z-axis sensor touched something
    char claw_holds_obj; // = claw_enaged && claw_cargo_touched ;) easing life
    char claw_working_layer; //-1 (too low), 0, 1, 2(float), -2 (currently invalid)

    int pos_logical_y; //-1 = moving, -2 = invalid
    int pos_logical_x; //-1 = moving, -2 = invalid
    
    char door_state; //-2 = invalid.. 0 closed (all fully), 1 openned (all fully), -1 - prob moving?
    
};

computed_states_t computed_states;

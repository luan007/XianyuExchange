#include "motion.h"

#define TODO(q) PLEASE_NOTE(q)

#define CODE_DOOR_STATE 0
#define CODE_CLAW_STATE 1
#define CODE_CLAW_GRAB_STATE 2
#define CODE_LOGICAL_Z_STATE 3

#define UNKNOWN -2
#define SET_UNKNOWN(q, code) q = -2;

#define CLAW_LAYERS 2 //(equals claw_float height)
#define CLAW_FLOAT CLAW_LAYERS

#define CLAW_HOLDING_POSITION = 3000;

#define Z_LOGICAL_ERROR_RANGE 300 //+/- 300 is acceptable

typedef struct computed_sensory_t;
{
    //---physical input sensors
    char s_claw_cargo_touched = 0;
    char s_delivery_outside = 0;
    char s_delivery_inside = 0;
    char s_delivery_has_content = 0;
    char s_door_closed = 0;
    char s_door_open = 0;

    //---computed sensors
    char xy_lock = 0;   //if z is uncertain or z is too low - lock xy
    char z_lock = 0;    //if xy is moving - do not move z
    long z_range = 0;   //we need to limit z-range for safety reasons. i.e, while logical_x, y does not fit (moving)
    char claw_lock = 0; //when ever xy're moving - lock claw

    //claw machanics
    char claw_engaged = -2; //0 no, 1 yes, -2 = invalid
    char claw_grabbed = -2; // = claw_enaged && claw_cargo_touched ;) easing life - trust only 1, 0 = useless

    int xy_logical_x = -2; //-1 = moving, -2 = invalid
    int xy_logical_y = -2; //-1 = moving, -2 = invalid
    int xy_logical_z = -2; //-1 (too low), 0, 1, 2(float), -2 (currently invalid)

    char delivery_box_position = -1; //-1 = unknown, 0 = inside, 1 = outside
    char door_state = -2;            //-2 = invalid.. 0 closed (all fully), 1 openned (all fully), -1 - prob moving?

    const int xy_logical_z_map[] = {20000, 15000, 10000, 0}; //too low!!, layer0 target, layer1 target, safe height
};

computed_sensory_t computed_sensor;

void tick_sensors()
{
    //read all sensors

    //digitalread everything..

    //compute logical xyz as many relates to them
    TODO("compute logical x")
    TODO("compute logical y")
    if (check_motor_flag(motors[STEPPER_Z], MOTOR_MOVING))
    {
        SET_UNKNOWN(computed_sensor.xy_logical_z, CODE_LOGICAL_Z_STATE) //moving & not trustworthy
    }
    else if (!check_motor_flag(motors[STEPPER_Z], MOTOR_CLEAN))
    {
        SET_UNKNOWN(computed_sensor.xy_logical_z, CODE_LOGICAL_Z_STATE) //moving & not trustworthy
    }
    //stopped and clean, wow
    else
    {
        //compute height
        if (motors[STEPPER_Z].position <= computed_sensor.xy_logical_z_map[CLAW_LAYERS + 1])
        { //safe flight. (super high)
            computed_sensor.xy_logical_z = CLAW_FLOAT;
        }
        else if (motors[STEPPER_Z].position >= computed_sensor.xy_logical_z_map[0]) //super impossible low
        {
            computed_sensor.xy_logical_z = -1;
        }
        else
        {
            unsigned char _set = -2;
            //calculate designated position
            for (unsigned char i = 0; i < CLAW_LAYERS; i++)
            {
                if (abs(motors[STEPPER_Z].position - computed_sensor.xy_logical_z_map[i + 1]) < Z_LOGICAL_ERROR_RANGE)
                {
                    _set = i;
                    break;
                }
            }
            computed_sensor.xy_logical_z = _set;
        }
    }

    //we need to sanitize motorZ's max range
    //i.e do not push when you hit something without clawing
    if (check_motor_flag(motors[STEPPER_X], MOTOR_MOVING) ||
        check_motor_flag(motors[STEPPER_Y], MOTOR_MOVING))
    {
        //xy is moving, for safety we must fully
        computed_sensor.z_range = computed_sensor.xy_logical_z_map[CLAW_FLOAT];
        //this is actually not possible as z should be locked already
    } else if(computed_sensor.xy_logical_x < 0 || computed_sensor.xy_logical_y < 0) {
        computed_sensor.z_range = computed_sensor.xy_logical_z_map[CLAW_FLOAT];
    } else if(computed_sensor.s_claw_cargo_touched && computed_sensor.claw_grabbed != 1) {
        computed_sensor.z_range = motors[STEPPER_Z].position; //do not push down anymore when we touched anything & not grabbed yet
    }

    //compute stepper locks
    computed_sensor.xy_lock = (check_motor_flag(motors[STEPPER_Z], MOTOR_MOVING) ||
                               (!check_motor_flag(motors[STEPPER_Z], MOTOR_CLEAN)) ||
                               computed_sensor.xy_logical_z < CLAW_LAYERS) //too low
                                  ? 1
                                  : 0;

    computed_sensor.z_lock = (check_motor_flag(motors[STEPPER_X], MOTOR_MOVING) ||
                              check_motor_flag(motors[STEPPER_Y], MOTOR_MOVING))
                                 ? 1
                                 : 0;

    computed_sensor.claw_lock =
        (check_motor_flag(motors[STEPPER_X], MOTOR_MOVING) ||
         check_motor_flag(motors[STEPPER_Y], MOTOR_MOVING) ||
         check_motor_flag(motors[STEPPER_Z], MOTOR_MOVING))
            ? 1
            : 0;

    //compute door state
    if (computed_sensor.s_door_closed && !computed_sensor.s_door_open)
    {
        computed_sensor.door_state = 0;
    }
    else if (!computed_sensor.s_door_closed && computed_sensor.s_door_open)
    {
        computed_sensor.door_state = 1;
    }
    else if (
        !computed_sensor.s_door_closed &&
        !computed_sensor.s_door_open &&
        check_motor_flag(motors[STEPPER_DOOR], MOTOR_MOVING))
    {
        computed_sensor.door_state = -1; //door is moving
    }
    else
    {
        //error..
        SET_UNKNOWN(computed_sensor.door_state, CODE_DOOR_STATE) //warning
    }

    //compute claw state
    if ((check_motor_flag(motors[STEPPER_CLAW_R], MOTOR_MOVING)) ||
        (check_motor_flag(motors[STEPPER_CLAW_L], MOTOR_MOVING)))
    {
        computed_sensor.claw_engaged = -1; //moving - do not trust
    }
    else if (
        check_motor_flag(motors[STEPPER_CLAW_L], MOTOR_CLEAN) &&
        check_motor_flag(motors[STEPPER_CLAW_R], MOTOR_CLEAN) &&
        motors[STEPPER_CLAW_L].position >= CLAW_HOLDING_POSITION &&
        motors[STEPPER_CLAW_R].position >= CLAW_HOLDING_POSITION)
    {
        computed_sensor.claw_engaged = 1;
    }
    else if (
        check_motor_flag(motors[STEPPER_CLAW_L], MOTOR_CLEAN) &&
        check_motor_flag(motors[STEPPER_CLAW_R], MOTOR_CLEAN) &&
        motors[STEPPER_CLAW_L].position <= 1 &&
        motors[STEPPER_CLAW_R].position <= 1 //at init pos
    )
    {
        computed_sensor.claw_engaged = 0;
    }
    else
    {
        SET_UNKNOWN(computed_sensor.claw_engaged, CODE_CLAW_STATE) //maybe locked due to some reason - like z is adjusting
    }

    //compute grab
    if (!computed_sensor.s_claw_cargo_touched)
    {
        computed_sensor.claw_grabbed = 0; //there is nothing underneath! - this is a fully released state
    }
    else if (computed_sensor.claw_enaged < 0) //there's something detected - but claw is moving or locked due to unknown reason
    {
        computed_sensor.claw_grabbed = -1; //grab process = moving - do not trust
    }
    else if (computed_sensor.claw_enaged == 0)
    {
        computed_sensor.claw_grabbed = 0; //there's something but - claw is released & ready to go
    }
    else if (computed_sensor.claw_enaged == 1)
    {
        computed_sensor.claw_grabbed = 1; //yay fully grabbed
    }
    else
    {
        SET_UNKNOWN(computed_sensor.claw_grabbed, CODE_CLAW_GRAB_STATE) //what?
    }

    ensure_locks();
}

void ensure_locks()
{
    computed_sensor.claw_lock ? enable_motor_flag(motors[STEPPER_CLAW_R], MOTOR_LOCK) : disable_motor_flag(motors[STEPPER_CLAW_R], MOTOR_LOCK);
    computed_sensor.claw_lock ? enable_motor_flag(motors[STEPPER_CLAW_L], MOTOR_LOCK) : disable_motor_flag(motors[STEPPER_CLAW_L], MOTOR_LOCK);
    computed_sensor.xy_lock ? enable_motor_flag(motors[STEPPER_X], MOTOR_LOCK) : disable_motor_flag(motors[STEPPER_X], MOTOR_LOCK);
    computed_sensor.xy_lock ? enable_motor_flag(motors[STEPPER_Y], MOTOR_LOCK) : disable_motor_flag(motors[STEPPER_Y], MOTOR_LOCK);
    computed_sensor.z_lock ? enable_motor_flag(motors[STEPPER_Z], MOTOR_LOCK) : disable_motor_flag(motors[STEPPER_Z], MOTOR_LOCK);
}
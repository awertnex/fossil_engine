#include "h/input.h"
#include "h/time.h"

static u32 mouse_button[MOUSE_BUTTONS_MAX] = {0};
static u32 keyboard_key[KEYBOARD_KEYS_MAX] = {0};
static u32 keyboard_tab[KEYBOARD_KEYS_MAX] =
{
    GLFW_KEY_SPACE,
    GLFW_KEY_APOSTROPHE,
    GLFW_KEY_COMMA,
    GLFW_KEY_MINUS,
    GLFW_KEY_PERIOD,
    GLFW_KEY_SLASH,
    GLFW_KEY_0,
    GLFW_KEY_1,
    GLFW_KEY_2,
    GLFW_KEY_3,
    GLFW_KEY_4,
    GLFW_KEY_5,
    GLFW_KEY_6,
    GLFW_KEY_7,
    GLFW_KEY_8,
    GLFW_KEY_9,
    GLFW_KEY_SEMICOLON,
    GLFW_KEY_EQUAL,
    GLFW_KEY_A,
    GLFW_KEY_B,
    GLFW_KEY_C,
    GLFW_KEY_D,
    GLFW_KEY_E,
    GLFW_KEY_F,
    GLFW_KEY_G,
    GLFW_KEY_H,
    GLFW_KEY_I,
    GLFW_KEY_J,
    GLFW_KEY_K,
    GLFW_KEY_L,
    GLFW_KEY_M,
    GLFW_KEY_N,
    GLFW_KEY_O,
    GLFW_KEY_P,
    GLFW_KEY_Q,
    GLFW_KEY_R,
    GLFW_KEY_S,
    GLFW_KEY_T,
    GLFW_KEY_U,
    GLFW_KEY_V,
    GLFW_KEY_W,
    GLFW_KEY_X,
    GLFW_KEY_Y,
    GLFW_KEY_Z,
    GLFW_KEY_LEFT_BRACKET,
    GLFW_KEY_BACKSLASH,
    GLFW_KEY_RIGHT_BRACKET,
    GLFW_KEY_GRAVE_ACCENT,
    GLFW_KEY_WORLD_1,
    GLFW_KEY_WORLD_2,

    GLFW_KEY_ESCAPE,
    GLFW_KEY_ENTER,
    GLFW_KEY_TAB,
    GLFW_KEY_BACKSPACE,
    GLFW_KEY_INSERT,
    GLFW_KEY_DELETE,
    GLFW_KEY_RIGHT,
    GLFW_KEY_LEFT,
    GLFW_KEY_DOWN,
    GLFW_KEY_UP,
    GLFW_KEY_PAGE_UP,
    GLFW_KEY_PAGE_DOWN,
    GLFW_KEY_HOME,
    GLFW_KEY_END,
    GLFW_KEY_CAPS_LOCK,
    GLFW_KEY_SCROLL_LOCK,
    GLFW_KEY_NUM_LOCK,
    GLFW_KEY_PRINT_SCREEN,
    GLFW_KEY_PAUSE,
    GLFW_KEY_F1,
    GLFW_KEY_F2,
    GLFW_KEY_F3,
    GLFW_KEY_F4,
    GLFW_KEY_F5,
    GLFW_KEY_F6,
    GLFW_KEY_F7,
    GLFW_KEY_F8,
    GLFW_KEY_F9,
    GLFW_KEY_F10,
    GLFW_KEY_F11,
    GLFW_KEY_F12,
    GLFW_KEY_F13,
    GLFW_KEY_F14,
    GLFW_KEY_F15,
    GLFW_KEY_F16,
    GLFW_KEY_F17,
    GLFW_KEY_F18,
    GLFW_KEY_F19,
    GLFW_KEY_F20,
    GLFW_KEY_F21,
    GLFW_KEY_F22,
    GLFW_KEY_F23,
    GLFW_KEY_F24,
    GLFW_KEY_F25,
    GLFW_KEY_KP_0,
    GLFW_KEY_KP_1,
    GLFW_KEY_KP_2,
    GLFW_KEY_KP_3,
    GLFW_KEY_KP_4,
    GLFW_KEY_KP_5,
    GLFW_KEY_KP_6,
    GLFW_KEY_KP_7,
    GLFW_KEY_KP_8,
    GLFW_KEY_KP_9,
    GLFW_KEY_KP_DECIMAL,
    GLFW_KEY_KP_DIVIDE,
    GLFW_KEY_KP_MULTIPLY,
    GLFW_KEY_KP_SUBTRACT,
    GLFW_KEY_KP_ADD,
    GLFW_KEY_KP_ENTER,
    GLFW_KEY_KP_EQUAL,
    GLFW_KEY_LEFT_SHIFT,
    GLFW_KEY_LEFT_CONTROL,
    GLFW_KEY_LEFT_ALT,
    GLFW_KEY_LEFT_SUPER,
    GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_RIGHT_CONTROL,
    GLFW_KEY_RIGHT_ALT,
    GLFW_KEY_RIGHT_SUPER,
    GLFW_KEY_MENU,
}; /* keyboard_tab */

void update_mouse_movement(void)
{
    static v2f64 mouse_last = {0};
    glfwGetCursorPos(render->window, &render->mouse_pos.x, &render->mouse_pos.y);
    render->mouse_delta = (v2f64){
        render->mouse_pos.x - mouse_last.x,
        render->mouse_pos.y - mouse_last.y,
    };
    mouse_last = render->mouse_pos;
}

b8 is_mouse_press(const u32 button)
{
    return mouse_button[button] == KEY_PRESS;
}

b8 is_mouse_hold(const u32 button)
{
    return mouse_button[button] == KEY_HOLD;
}

b8 is_mouse_release(const u32 button)
{
    return mouse_button[button] == KEY_RELEASE;
}

b8 is_key_press(const u32 key)
{
    return keyboard_key[key] == KEY_PRESS ||
        keyboard_key[key] == KEY_PRESS_DOUBLE;
}

b8 is_key_press_double(const u32 key)
{
    return keyboard_key[key] == KEY_PRESS_DOUBLE;
}

b8 is_key_hold(const u32 key)
{
    return keyboard_key[key] == KEY_HOLD ||
        keyboard_key[key] == KEY_HOLD_DOUBLE;
}

b8 is_key_release(const u32 key)
{
    return keyboard_key[key] == KEY_RELEASE ||
        keyboard_key[key] == KEY_RELEASE_DOUBLE;
}

void update_key_states(void)
{
    GLFWwindow *_window = render->window;
    u64 _time = render->time;
    static u64 double_press_time_interval = (u64)(DOUBLE_PRESS_TIME_INTERVAL * SEC2NSEC);
    static u64 key_press_start_time[KEYBOARD_KEYS_MAX] = {0};
    b8 key_press = FALSE, key_release = FALSE,
       mouse_press = FALSE, mouse_release = FALSE;
    u32 i;

    for (i = 0; i < MOUSE_BUTTONS_MAX; ++i)
    {
        mouse_press = glfwGetMouseButton(_window, i) == GLFW_PRESS;
        mouse_release = glfwGetMouseButton(_window, i) == GLFW_RELEASE;

        if (mouse_press &&
                (mouse_button[i] == KEY_IDLE))
        {
            mouse_button[i] = KEY_PRESS;
            continue;
        }
        else if (mouse_release &&
                (mouse_button[i] == KEY_PRESS || mouse_button[i] == KEY_HOLD))
        {
            mouse_button[i] = KEY_RELEASE;
            continue;
        }

        if (mouse_button[i] == KEY_PRESS)           mouse_button[i] = KEY_HOLD;
        else if (mouse_button[i] == KEY_RELEASE)    mouse_button[i] = KEY_IDLE;
    }

    for (i = 0; i < KEYBOARD_KEYS_MAX; ++i)
    {
        key_press = glfwGetKey(_window, keyboard_tab[i]) == GLFW_PRESS;
        key_release = glfwGetKey(_window, keyboard_tab[i]) == GLFW_RELEASE;

        if (key_press)
        {
            if (keyboard_key[i] == KEY_IDLE)
            {
                keyboard_key[i] = KEY_PRESS;
                key_press_start_time[i] = _time;
                continue;
            }
            else if (keyboard_key[i] == KEY_LISTEN_DOUBLE)
            {
                if (_time - key_press_start_time[i] <= double_press_time_interval)
                    keyboard_key[i] = KEY_PRESS_DOUBLE;
                else
                {
                    keyboard_key[i] = KEY_PRESS;
                    key_press_start_time[i] = _time;
                }
                continue;
            }
        }
        else if (key_release)
        {
            if (keyboard_key[i] == KEY_PRESS ||
                    keyboard_key[i] == KEY_HOLD)
            {
                keyboard_key[i] = KEY_RELEASE;
                continue;
            }
            else if (keyboard_key[i] == KEY_PRESS_DOUBLE ||
                    keyboard_key[i] == KEY_HOLD_DOUBLE)
            {
                keyboard_key[i] = KEY_RELEASE_DOUBLE;
                continue;
            }
        }

        if (keyboard_key[i] == KEY_PRESS)               keyboard_key[i] = KEY_HOLD;
        else if (keyboard_key[i] == KEY_RELEASE)        keyboard_key[i] = KEY_LISTEN_DOUBLE;
        if (keyboard_key[i] == KEY_PRESS_DOUBLE)        keyboard_key[i] = KEY_HOLD_DOUBLE;
        else if (keyboard_key[i] == KEY_RELEASE_DOUBLE) keyboard_key[i] = KEY_IDLE;
    }
}

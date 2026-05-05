#include "button.h"
#include "hal/printf_selector.h"
#include "hal/tasks.h"
#include "hal/timer.h"
#include <stdbool.h>
#include <stddef.h>

void _btn_poll_task(void *arg);
void _btn_update_callback(void *arg);
void btn_update_debounced(button_t *button, uint8_t is_pressed,
                          uint32_t changed_at);

void btn_init(button_t *button) {
    // During device startup, button may be already pressed, but this should not
    // be detected as user press. So, to avoid such situation, special init is
    // required.
    uint8_t state = hal_gpio_read(button->pin);

    if (button->max_integrator == 0) {
        button->max_integrator = button->debounce_delay_ms / 5;
        if (button->max_integrator == 0) {
            button->max_integrator = 10;
        }
    }

    if (state == button->pressed_when_high) {
        button->pressed      = true;
        button->long_pressed = true;
    }
    button->debounce_last_state = state;
    button->integrator = (state == button->pressed_when_high) ? button->max_integrator : 0;

    button->update_task.handler = _btn_update_callback;
    button->update_task.arg     = button;
    hal_tasks_init(&button->update_task);

    button->poll_task.handler = _btn_poll_task;
    button->poll_task.arg     = button;
    hal_tasks_init(&button->poll_task);
    hal_tasks_schedule(&button->poll_task, 5);
}

void _btn_poll_task(void *arg) {
    button_t *button = (button_t *)arg;
    uint8_t   raw_state = hal_gpio_read(button->pin);
    uint8_t   is_active = (raw_state == button->pressed_when_high);

    if (is_active) {
        if (button->integrator < button->max_integrator) {
            button->integrator++;
        }
    } else {
        if (button->integrator > 0) {
            button->integrator--;
        }
    }

    uint8_t new_state = button->debounce_last_state;
    if (button->integrator == button->max_integrator) {
        new_state = button->pressed_when_high;
    } else if (button->integrator == 0) {
        new_state = !button->pressed_when_high;
    }

    if (new_state != button->debounce_last_state) {
        button->debounce_last_state  = new_state;
        button->debounce_last_change = hal_millis();
        hal_tasks_schedule(&button->update_task, 0);
    }

    hal_tasks_schedule(&button->poll_task, 5);
}

void _btn_update_callback(void *arg) {
    button_t *button = (button_t *)arg;

    btn_update_debounced(button,
                         button->debounce_last_state == button->pressed_when_high,
                         button->debounce_last_change);
    if (button->pressed && !button->long_pressed) {
        uint32_t pressed_for = hal_millis() - button->pressed_at_ms;
        hal_tasks_schedule(&button->update_task,
                           pressed_for < button->long_press_duration_ms
                           ? button->long_press_duration_ms - pressed_for
                           : 0);
    }
}

void btn_update_debounced(button_t *button, uint8_t is_pressed,
                          uint32_t changed_at) {
    if (!button->pressed && is_pressed) {
        printf("Press detected\r\n");
        button->pressed_at_ms = changed_at;
        button->pressed       = true;
        if (button->on_press != NULL) {
            button->on_press(button->callback_param);
        }
        if (changed_at - button->released_at_ms < button->multi_press_duration_ms) {
            button->multi_press_cnt += 1;
            printf("Multi press detected: %d\r\n", button->multi_press_cnt);
            if (button->on_multi_press != NULL) {
                button->on_multi_press(button->callback_param, button->multi_press_cnt);
            }
        } else {
            button->multi_press_cnt = 1;
        }
    } else if (button->pressed && !is_pressed) {
        printf("Release detected\r\n");
        button->released_at_ms = changed_at;
        button->pressed        = false;
        button->long_pressed   = false;
        if (button->on_release != NULL) {
            button->on_release(button->callback_param);
        }
    }
    button->pressed = is_pressed;

    uint32_t now = hal_millis();
    if (is_pressed && !button->long_pressed &&
        (button->long_press_duration_ms <= (now - button->pressed_at_ms))) {
        button->long_pressed = true;
        printf("Long press detected\r\n");
        if (button->on_long_press != NULL) {
            button->on_long_press(button->callback_param);
        }
    }
}

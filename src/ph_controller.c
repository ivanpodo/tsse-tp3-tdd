/* ======================= Headers files inclusions ============================================= */
#include "ph_controller.h"
#include <stddef.h>
#include "stdio.h"

/* ======================= Macros definitions =================================================== */
#define DEFAULT_SETPOINT          6.4f
#define DEFAULT_PROPORTIONAL_GAIN 1.0f
#define DEFAULT_CURRENT_VALUE_MAX 7.5f
#define DEFAULT_CURRENT_VALUE_MIN 6.0f
#define DEFAULT_OUTPUT_VALUE_MIN  15
#define OUTPUT_VALUE_MAX_F        100.0f

#ifdef DEBUG
#define ASSERT(cond)   \
    do {               \
        if (!(cond)) { \
            while (1)  \
                ;      \
        }              \
    } while (0)
#else
#define ASSERT(cond) ((void)0)
#endif
/* ======================= Private data type declarations ======================================= */
/* ======================= Private variable declarations ======================================== */
/* ======================= Private function declarations ======================================== */
ph_controller_error_t _validate_config(const ph_controller_config_t *config);
int _transform_output_value(const float raw_value, const int output_value_min);
/* ======================= Private variable definitions ========================================= */
/* ======================= Private function implementation ====================================== */
ph_controller_error_t _validate_config(const ph_controller_config_t *config) {
    ASSERT(NULL != config);

    if ((config->current_value_max - config->current_value_min) < 0.0f) {
        return (ph_controller_error_t)BAD_CURRENT_VALUE_LIMITS;
    }

    if ((config->current_value_max - config->setpoint_value) < 0.0f ||
        (config->current_value_min - config->setpoint_value) > 0.0f) {
        return (ph_controller_error_t)SETPOINT_OUT_OF_RANGE;
    }

    if (0.0f > (config->proportional_gain - 0.01f)) {
        return (ph_controller_error_t)INVALID_GAIN;
    }

    return (ph_controller_error_t)CONTROLLER_OK;
}

int _transform_output_value(const float raw_value, const int output_value_min) {
    ASSERT((OUTPUT_VALUE_MAX_F - (float)output_value_min) > 0.0);

    if (OUTPUT_VALUE_MAX_F <= raw_value) {
        return (int)OUTPUT_VALUE_MAX_F;
    }

    if ((float)output_value_min >= raw_value) {
        return output_value_min;
    }

    return (int)raw_value;
}

/* ======================= Public function implementation ======================================= */
ph_controller_error_t ph_controller_create(ph_controller_t *controller,
                                           const ph_controller_config_t *config) {
    ASSERT(NULL != config);
    ASSERT(NULL != controller);

    controller->state = (ph_controller_state_t)STOPPED;

    ph_controller_error_t _errno = ph_controller_set_configuration(controller, config);

    if ((ph_controller_error_t)CONTROLLER_OK != _errno) {
        return _errno;
    }

    return _errno;
}

ph_controller_error_t ph_controller_start(ph_controller_t *controller) {
    ASSERT(NULL != controller);

    ph_controller_error_t _errno = (ph_controller_error_t)GENERIC_ERROR;
    _errno = _validate_config(&(controller->config));

    if ((ph_controller_error_t)CONTROLLER_OK != _errno) {
        return _errno;
    }

    controller->state = RUNNING;
    return _errno;
}

ph_controller_error_t ph_controller_stop(ph_controller_t *controller) {
    ASSERT(NULL != controller);

    controller->state = STOPPED;
    return (ph_controller_error_t)CONTROLLER_OK;  // Más explícito que !controller->state
}

ph_controller_error_t ph_controller_set_configuration(ph_controller_t *controller,
                                                      const ph_controller_config_t *config) {
    ASSERT(NULL != config);
    ASSERT(NULL != controller);

    ph_controller_error_t _errno = (ph_controller_error_t)GENERIC_ERROR;

    if ((ph_controller_state_t)RUNNING == controller->state) {
        _errno = (ph_controller_error_t)CONTROLLER_IS_RUNNING;
        return _errno;
    }

    _errno = _validate_config(config);

    if ((ph_controller_error_t)CONTROLLER_OK != _errno) {
        return _errno;
    }

    controller->config = *config;
    return _errno;
}

void ph_controller_get_default_config(ph_controller_config_t *config) {
    ASSERT(NULL != config);

    config->setpoint_value = DEFAULT_SETPOINT;
    config->proportional_gain = DEFAULT_PROPORTIONAL_GAIN;
    config->current_value_max = DEFAULT_CURRENT_VALUE_MAX;
    config->current_value_min = DEFAULT_CURRENT_VALUE_MIN;
    config->output_value_min = DEFAULT_OUTPUT_VALUE_MIN;
}

ph_controller_error_t ph_controller_update(ph_controller_t *controller, const float current_value) {
    ASSERT(NULL != controller);
    ASSERT(NULL != config);

    ph_controller_error_t _errno = (ph_controller_error_t)GENERIC_ERROR;

    if ((ph_controller_state_t)STOPPED == controller->state) {
        _errno = (ph_controller_error_t)CONTROLLER_STOPPED;
        return _errno;
    }

    if ((current_value - controller->config.setpoint_value) < 0.0f) {
        _errno = (ph_controller_error_t)CURRENT_VALUE_TOO_LOW;
        return _errno;
    }

    // Calculo de error
    float _controller_error = current_value - controller->config.setpoint_value;
    // Pre-cálculo del valor de la salida
    float _output_raw = controller->config.proportional_gain * _controller_error;
    // Cálculo del valor de la salida
    controller->output_value =
        _transform_output_value(_output_raw, controller->config.output_value_min);
    _errno = (ph_controller_error_t)CONTROLLER_OK;

    return _errno;
}

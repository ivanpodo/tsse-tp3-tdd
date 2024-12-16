#include "unity.h"
#include "ph_controller.h"

ph_controller_t ph_controller;

// Se ejecuta antes de cada prueba
void setUp(void) {
    ph_controller_config_t config;
    ph_controller_get_default_config(&config);
    ph_controller_create(&ph_controller, &config);
}

// Se ejecuta luego de cada prueba
void tearDown(void) {
}

// 1 -El módulo se debe poder iniciar.
void test_inicio_del_modulo(void) {
    ph_controller_config_t config;
    ph_controller_get_default_config(&config);
    config.setpoint_value = 15.0f;
    TEST_ASSERT_EQUAL((ph_controller_error_t)SETPOINT_OUT_OF_RANGE,
                      ph_controller_create(&ph_controller, &config));
    ph_controller_get_default_config(&config);
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK, ph_controller_start(&ph_controller));
}

// 2. El módulo se debe poder parar.
void test_parar_el_modulo(void) {
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK, ph_controller_stop(&ph_controller));
}

// 3. En el módulo se debe poder configurar
void test_configurar_modulo(void) {
    ph_controller_config_t new_config;
    ph_controller_get_default_config(&new_config);
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK,
                      ph_controller_set_configuration(&ph_controller, &new_config));
}

// 4. En el módulo debe validar la configuración
// 5. El módulo debe indicar si falla la configuración, indicando el motivo.
void test_validar_configuracion_de_modulo(void) {
    ph_controller_config_t new_config;
    ph_controller_get_default_config(&new_config);
    // 1. current_value_min < current_value_max
    new_config.current_value_min = 7.5;
    new_config.current_value_max = 6.4;
    TEST_ASSERT_EQUAL((ph_controller_error_t)BAD_CURRENT_VALUE_LIMITS,
                      ph_controller_set_configuration(&ph_controller, &new_config));
    new_config.current_value_min = 6.4;
    new_config.current_value_max = 7.5;
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK,
                      ph_controller_set_configuration(&ph_controller, &new_config));

    // 2. current_value_min <= setpoint <= current_value_max
    ph_controller_get_default_config(&new_config);
    new_config.setpoint_value = new_config.current_value_max + 0.1;
    TEST_ASSERT_EQUAL((ph_controller_error_t)SETPOINT_OUT_OF_RANGE,
                      ph_controller_set_configuration(&ph_controller, &new_config));
    new_config.setpoint_value = new_config.current_value_min - 0.1;
    TEST_ASSERT_EQUAL((ph_controller_error_t)SETPOINT_OUT_OF_RANGE,
                      ph_controller_set_configuration(&ph_controller, &new_config));
    new_config.setpoint_value = new_config.current_value_min + 0.1;
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK,
                      ph_controller_set_configuration(&ph_controller, &new_config));

    // 3. proportional_gain > 0
    ph_controller_get_default_config(&new_config);
    new_config.proportional_gain = 0.0f;
    TEST_ASSERT_EQUAL((ph_controller_error_t)INVALID_GAIN,
                      ph_controller_set_configuration(&ph_controller, &new_config));
    new_config.proportional_gain = (-1.0f);
    TEST_ASSERT_EQUAL((ph_controller_error_t)INVALID_GAIN,
                      ph_controller_set_configuration(&ph_controller, &new_config));
    new_config.proportional_gain = 1.0f;
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK,
                      ph_controller_set_configuration(&ph_controller, &new_config));
}

// 6. Antes de iniciar, el módulo debe ser configurado correctamente.
void test_modulo_correctamente_configurado_antes_de_iniciar(void) {
    ph_controller_config_t new_config;
    ph_controller_get_default_config(&new_config);  // Guardo una configuracion válida
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK,
                      ph_controller_set_configuration(&ph_controller, &new_config));

    ph_controller.config.setpoint_value = ph_controller.config.current_value_min - 0.1f;
    TEST_ASSERT_EQUAL((ph_controller_error_t)SETPOINT_OUT_OF_RANGE,
                      ph_controller_start(&ph_controller));

    ph_controller_set_configuration(&ph_controller, &new_config);  // Configuro una conf válida
    ph_controller.config.current_value_max = ph_controller.config.current_value_min - 0.1f;
    TEST_ASSERT_EQUAL((ph_controller_error_t)BAD_CURRENT_VALUE_LIMITS,
                      ph_controller_start(&ph_controller));

    ph_controller_set_configuration(&ph_controller, &new_config);  // Configuro una conf válida
    ph_controller.config.proportional_gain = 0.0f;
    TEST_ASSERT_EQUAL((ph_controller_error_t)INVALID_GAIN, ph_controller_start(&ph_controller));

    ph_controller_set_configuration(&ph_controller, &new_config);
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK, ph_controller_start(&ph_controller));
}

// 7. Sólo se podrá cambiar la configuración del módulo cuando el mismo no esté controlando
void test_cambio_configuracion_en_caliente(void) {
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK, ph_controller_start(&ph_controller));
    ph_controller_config_t new_config;
    ph_controller_get_default_config(&new_config);
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_IS_RUNNING,
                      ph_controller_set_configuration(&ph_controller, &new_config));
    ph_controller_stop(&ph_controller);
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK,
                      ph_controller_set_configuration(&ph_controller, &new_config));
}

// 8/9/10/11. Prueba de la salida del controlador
void test_calculo_de_la_salida_del_controlador(void) {
    TEST_ASSERT_EQUAL(
        (ph_controller_error_t)CONTROLLER_STOPPED,
        ph_controller_update(&ph_controller, ph_controller.config.setpoint_value - 1.0f));
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK, ph_controller_start(&ph_controller));
    TEST_ASSERT_EQUAL(
        (ph_controller_error_t)CURRENT_VALUE_TOO_LOW,
        ph_controller_update(&ph_controller, ph_controller.config.setpoint_value - 0.1f));
    // Error pequeño (< 15%)
    TEST_ASSERT_EQUAL(
        (ph_controller_error_t)CONTROLLER_OK,
        ph_controller_update(&ph_controller, ph_controller.config.setpoint_value + 1.0f));
    // Error moderado (20%)
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK, ph_controller_stop(&ph_controller));
    ph_controller.config.proportional_gain = 20.0f;
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK, ph_controller_start(&ph_controller));
    TEST_ASSERT_EQUAL(
        (ph_controller_error_t)CONTROLLER_OK,
        ph_controller_update(&ph_controller, ph_controller.config.setpoint_value + 1.0f));
    // Error muy elevado (> 100%)
    ph_controller.config.proportional_gain = 50.0f;
    TEST_ASSERT_EQUAL((ph_controller_error_t)CONTROLLER_OK, ph_controller_start(&ph_controller));
    TEST_ASSERT_EQUAL(
        (ph_controller_error_t)CONTROLLER_OK,
        ph_controller_update(&ph_controller, ph_controller.config.setpoint_value + 3.0f));
}

/**
 * @file ph_controller.h
 * @brief Controlador de pH basado en PWM para bombas peristálticas.
 *
 * Este archivo contiene las definiciones y funciones necesarias para
 * configurar y controlar un sistema de regulación de pH. Permite manejar
 * configuraciones, iniciar y detener el controlador, y actualizar los valores
 * basados en las mediciones actuales.
 *
 * @author Iván Podoroska
 */

#ifndef PH_CONTROLLER_H
#define PH_CONTROLLER_H

#include <stdint.h>
#include "stdbool.h"

// #define DEBUG /**< Habilitar para debug */

/**
 * @enum ph_controller_error_t
 * @brief Errores del controlador de pH.
 */
typedef enum {
    CONTROLLER_OK = 0,        /**< Operación exitosa. */
    BAD_CURRENT_VALUE_LIMITS, /**< Límites de valor actuales inválidos. */
    SETPOINT_OUT_OF_RANGE,    /**< El setpoint está fuera del rango permitido. */
    INVALID_GAIN,             /**< La ganancia proporcional es inválida. */
    CONTROLLER_IS_RUNNING,    /**< El controlador ya está en ejecución. */
    CONTROLLER_STOPPED,       /**< El controlador está detenido. */
    CURRENT_VALUE_TOO_LOW,    /**< Valor actual demasiado bajo. */
    GENERIC_ERROR,            /**< Error genérico. */
} ph_controller_error_t;

/**
 * @struct ph_controller_config_t
 * @brief Configuración del controlador de pH.
 */
typedef struct {
    float setpoint_value;    /**< Valor objetivo del pH. */
    float proportional_gain; /**< Ganancia proporcional del controlador. */
    float current_value_max; /**< Valor máximo permitido de pH. */
    float current_value_min; /**< Valor mínimo permitido de pH. */
    int output_value_min;    /**< Valor mínimo de la salida PWM. */
} ph_controller_config_t;

/**
 * @enum ph_controller_state_t
 * @brief Estados del controlador de pH.
 */
typedef enum {
    RUNNING = 0, /**< El controlador está en ejecución. */
    STOPPED,     /**< El controlador está detenido. */
} ph_controller_state_t;

/**
 * @struct ph_controller_t
 * @brief Estructura del controlador de pH.
 */
typedef struct {
    ph_controller_config_t config; /**< Configuración actual del controlador. */
    ph_controller_state_t state;   /**< Estado del controlador. */
    bool is_configured;            /**< Indicador de configuración válida. */
    int output_value;              /**< Valor actual de la salida PWM. */
} ph_controller_t;

/**
 * @brief Crea e inicializa un controlador de pH.
 *
 * @param controller Puntero al controlador a inicializar.
 * @param config Configuración inicial del controlador.
 * @return ph_controller_error_t Código de error correspondiente.
 */
ph_controller_error_t ph_controller_create(ph_controller_t *controller,
                                           const ph_controller_config_t *config);

/**
 * @brief Establece una nueva configuración para el controlador.
 *
 * @param controller Puntero al controlador.
 * @param config Nueva configuración a aplicar.
 * @return ph_controller_error_t Código de error correspondiente.
 */
ph_controller_error_t ph_controller_set_configuration(ph_controller_t *controller,
                                                      const ph_controller_config_t *config);

/**
 * @brief Inicia el controlador de pH.
 *
 * @param controller Puntero al controlador.
 * @return ph_controller_error_t Código de error correspondiente.
 */
ph_controller_error_t ph_controller_start(ph_controller_t *controller);

/**
 * @brief Detiene el controlador de pH.
 *
 * @param controller Puntero al controlador.
 * @return ph_controller_error_t Código de error correspondiente.
 */
ph_controller_error_t ph_controller_stop(ph_controller_t *controller);

/**
 * @brief Obtiene una configuración por defecto del controlador.
 *
 * @param config Puntero a la estructura donde se almacenará la configuración por defecto.
 */
void ph_controller_get_default_config(ph_controller_config_t *config);

/**
 * @brief Actualiza el estado del controlador en función del valor actual de pH.
 *
 * @param controller Puntero al controlador.
 * @param current_value Valor actual del pH.
 * @return ph_controller_error_t Código de error correspondiente.
 */
ph_controller_error_t ph_controller_update(ph_controller_t *controller, const float current_value);

#endif  // PH_CONTROLLER_H

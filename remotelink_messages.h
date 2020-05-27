/*
 * remotelink_messages.h
 *
 *  Created on: March. 2019
 *
 */


#ifndef RL_MESSAGES_H
#define RL_MESSAGES_H
//Codigos de los mensajes y definicion de parametros para el protocolo RPC

// El estudiante debe agregar aqui cada nuevo mensaje que implemente. IMPORTANTE el orden de los mensajes
// debe SER EL MISMO aqui, y en el codigo equivalente en la parte del microcontrolador (Code Composer)

typedef enum {
    MESSAGE_REJECTED,
    MESSAGE_PING,
    MESSAGE_LED_GPIO,
    MESSAGE_LED_PWM_BRIGHTNESS,
    MESSAGE_ADC_SAMPLE,
    MESSAGE_COLOR,
    MESSAGE_MODE,
    MESSAGE_BUTTON,
    MESSAGE_BUTTON_MODE,
    MESSAGE_ADC_MODE,
    MESSAGE_FACTOR,
    MESSAGE_TIMER_ADC,
    MESSAGE_RESOLUTION,
    MESSAGE_ADC8,
    MESSAGE_ADC12,
    MESSAGE_SIMULATION,
    MESSAGE_DATA_ADQ_MODE,
    MESSAGE_DATA_ADQ,
    MESSAGE_DATA_ADQ_CAPTURE,
    //etc, etc...
} messageTypes;

//Estructuras relacionadas con los parametros de los mensahes. El estuadiante debera crear las
// estructuras adecuadas a los comandos usados, y asegurarse de su compatibilidad en ambos extremos

#pragma pack(1) //Cambia el alineamiento de datos en memoria a 1 byte.

typedef struct {
    uint8_t command;
} MESSAGE_REJECTED_PARAMETER;

typedef union{
    struct {
         uint8_t padding:1;
         uint8_t red:1;
         uint8_t blue:1;
         uint8_t green:1;
    } leds;
    uint8_t value;
} MESSAGE_LED_GPIO_PARAMETER;

typedef struct {
    float rIntensity;
} MESSAGE_LED_PWM_BRIGHTNESS_PARAMETER;

typedef struct
{
    uint16_t chan1;
    uint16_t chan2;
    uint16_t chan3;
    uint16_t chan4;
    uint16_t chan5;
    uint16_t chan6;
}MESSAGE_ADC_SAMPLE_PARAMETER;

typedef struct
{
    uint8_t chan1;
    uint8_t chan2;
    uint8_t chan3;
    uint8_t chan4;
    uint8_t chan5;
    uint8_t chan6;
}MESSAGE_ADC8_PARAMETER;

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}MESSAGE_COLOR_PARAMETER;

typedef struct
{
    uint8_t index;
}MESSAGE_MODE_PARAMETER;

typedef struct
{
    bool left_button;
    bool right_button;
}MESSAGE_BUTTON_PARAMETER;

typedef struct
{
    bool mode;
}MESSAGE_BUTTON_MODE_PARAMETER;

typedef struct
{
    uint8_t index;
}MESSAGE_ADC_MODE_PARAMETER;

typedef struct
{
    uint32_t factor;
}MESSAGE_FACTOR_PARAMETER;

typedef struct
{
    bool on;
    uint32_t frecuencia;
}MESSAGE_TIMER_ADC_PARAMETER;

typedef struct
{
    bool resolution;
}MESSAGE_RESOLUTION_PARAMETER;

typedef struct
{
    bool simulation;
}MESSAGE_SIMULATION_PARAMETER;

typedef struct
{
    bool mode;
}MESSAGE_DATA_ADQ_MODE_PARAMETER;

typedef struct
{
    uint32_t muestras;
}MESSAGE_DATA_ADQ_PARAMETER;

typedef struct
{
    uint32_t frecuencia;
}MESSAGE_DATA_ADQ_CAPTURE_PARAMETER;
#pragma pack()  //...Pero solo para los comandos que voy a intercambiar, no para el resto.

#endif // RPCCOMMANDS_H



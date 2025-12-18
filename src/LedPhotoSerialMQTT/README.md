# Управление освещением с обменом данными по MQTT

Проект представляет собой распределённую IoT-систему для управления освещением. Два микроконтроллера обмениваются данными через MQTT-брокер, используя Python-скрипты на ПК для связи по Serial.

**Ссылка на Tinkercad:** [https://www.tinkercad.com/things/3lEG2WsQ2AY-ledphotoserialmqtt](https://www.tinkercad.com/things/3lEG2WsQ2AY-ledphotoserialmqtt)

## Архитектура

```
Sensor MCU (фоторезистор) ←UART→ PC1 (sensor_bridge.py) → MQTT Broker
                                                              ↓
Actuator MCU (LED) ←UART→ PC2 (actuator_bridge.py) ← MQTT Broker
                                                              ↓
                                                      Monitor (monitor.py)
```

## Компоненты

### 1. Sensor MCU (`firmware/SensorMCU/SensorMCU.ino`)

Читает данные с фоторезистора на пине A0 и отправляет их по Serial.

**Команды:**
- `p` - запросить одно значение
- `s` - начать потоковую передачу
- `q` - остановить поток

**Ответы:**
- `SENSOR_VALUE:<значение>` - значение с датчика (0-1023)
- `STREAM_STARTED` / `STREAM_STOPPED`

### 2. Actuator MCU (`firmware/ActuatorMCU/ActuatorMCU.ino`)

Управляет светодиодом на пине 13.

**Команды:**
- `u` - включить LED
- `d` - выключить LED
- `b` - мигать LED

**Ответы:**
- `LED_GOES_ON` / `LED_GOES_OFF` / `LED_WILL_BLINK`

### 3. Sensor Bridge (`software/sensor_bridge.py`)

Подключается к Sensor MCU по Serial, читает значения и публикует их в MQTT топик `laboratory/greenhouse/luminosity`.

**Настройка:** измените `SERIAL_PORT = 'COM8'` на ваш порт.

### 4. Actuator Bridge (`software/actuator_bridge.py`)

Подписывается на топик `laboratory/greenhouse/luminosity`, получает значения и управляет LED через Actuator MCU. Если значение < 500, включает LED, иначе выключает. Публикует статус в `laboratory/greenhouse/light_status`.

**Настройка:** измените `SERIAL_PORT = 'COM4'` на ваш порт.

### 5. Monitor (`software/monitor.py`)

Подписывается на все топики `laboratory/greenhouse/#` и выводит все сообщения с временными метками.

## Установка

1. Установите зависимости:
```bash
pip install -r requirements.txt
```

2. Загрузите прошивки на микроконтроллеры через Arduino IDE:
   - `firmware/SensorMCU/SensorMCU.ino` на первый MCU
   - `firmware/ActuatorMCU/ActuatorMCU.ino` на второй MCU

3. Определите Serial-порты:
   - Windows: Диспетчер устройств → Порты (COM и LPT)
   - Linux/Mac: `ls /dev/ttyUSB*` или `ls /dev/ttyACM*`

4. Укажите порты в скриптах `sensor_bridge.py` и `actuator_bridge.py`

## Запуск

1. Запустите монитор:
```bash
python software/monitor.py
```

2. Запустите Actuator Bridge:
```bash
python software/actuator_bridge.py
```

3. Запустите Sensor Bridge:
```bash
python software/sensor_bridge.py
```

## Логика работы

1. Sensor Bridge каждые 2 секунды отправляет команду `p` на Sensor MCU
2. Sensor MCU читает значение с фоторезистора и отправляет `SENSOR_VALUE:<value>`
3. Sensor Bridge публикует значение в MQTT топик `laboratory/greenhouse/luminosity`
4. Actuator Bridge получает значение из MQTT
5. Если значение < 500, отправляет команду `u` (включить), иначе `d` (выключить)
6. Actuator MCU выполняет команду и отправляет подтверждение
7. Actuator Bridge публикует статус в `laboratory/greenhouse/light_status`
8. Monitor отображает все сообщения

## Протоколы

**UART:** 9600 бод, 8N1

**MQTT:** 
- Брокер: `broker.emqx.io`
- QoS: 1
- Топики:
  - `laboratory/greenhouse/luminosity` - значения освещённости
  - `laboratory/greenhouse/light_status` - статус LED

## Тестирование

MCU-датчик отвечает на команду `p` значением  
MCU-датчик начинает поток по команде `s`  
MCU-исполнитель включает LED по команде `u`  
MCU-исполнитель выключает LED по команде `d`  
MCU-исполнитель мигает LED по команде `b`  
Все сообщения публикуются в MQTT  
Монитор отображает все события  
Система работает стабильно

## Структура проекта

```
ShiftRegistrersTimers/
├── firmware/
│   ├── SensorMCU/SensorMCU.ino
│   └── ActuatorMCU/ActuatorMCU.ino
├── software/
│   ├── sensor_bridge.py
│   ├── actuator_bridge.py
│   └── monitor.py
├── requirements.txt
└── README.md
```

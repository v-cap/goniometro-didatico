# Goniômetro Digital Didático

Projeto inicial de um goniômetro digital didático desenvolvido para leitura de ângulos do cotovelo utilizando o sensor MPU-6050.

## Descrição

Este repositório contém a primeira versão do código para um sistema de medição de flexão do cotovelo, com:

- leitura do acelerômetro e giroscópio do MPU-6050;
- calibração do sensor;
- integração angular por giroscópio;
- filtro complementar para estabilização;
- referência inicial para medir a flexão relativa;
- exibição do ângulo em graus pela porta serial.

O projeto está vinculado ao GEB e foi desenvolvido como base educacional para demonstração e estudo de sensores inerciais.

## Hardware

- Arduino Uno / compatível
- MPU-6050
- Conexão I2C padronizada (SDA/SCL)

## Arquivo principal

- `leitura_gy521_mpu6050_goniometro_didatico.ino`

## Como usar

1. Conecte o módulo MPU-6050 ao Arduino.
2. Carregue o sketch no microcontrolador.
3. Mantenha o sensor parado durante a calibração.
4. Posicione o braço e o antebraço conforme as instruções exibidas no monitor serial.
5. Pressione `z` para definir a referência inicial.
6. Inicie o movimento e acompanhe o ângulo calculado.

## Licença

Este projeto está licenciado sob a licença MIT.

void setPWM(){
    //configurar PWM
    PTCON0  = 0b00000000;   //pwm com pre/postscale 1:1, seleciona moda Free Running
    PTCON1  = 0b10000000;   //habilita base de tempo pwm
    PWMCON0 = 0b01011111;   //habilita todos pwm I/O como saída PWM e põe os pares no modo independente
    PWMCON1 = 0b00000001;
    DTCON   = 0b00010000;
    OVDCOND = 0b11111111;   //saída pwm controlada pelo valor de duty cicle e base de tempo pwm
    OVDCONS = 0b00000000;
    PTPERL  = 0b10010110;   //PTPERMAX=4*150=600 (velocidade máxima), para 4 MHz, PTPERMAX = 4*29 = 116
    PTPERH  = 0b00000000;
    PTMRL   = 0x00;         //base de tempo do pwm em 0
    PTMRH   = 0x00;
    PORTB   = 0;            //zera os valores das saídas PWM
    PORTD   = 0;
}


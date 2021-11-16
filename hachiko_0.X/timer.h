/*  Para 16 bits:
 *  3,4     = 3,4 - base desejada 
 *  65536   = x
 *  x = [(3,4 - base desejada)*65536]/3,4 
 * 
 * Para 8 bits
 * 13,1     = 13,1 - base desejada
 * 256      = x
 * x = [(13,1 - base desejada)*256]/13,1
 */

void setTMR0(){
    //configurar tmr0(registrador T0CON)
    TMR0ON      = 0;    //inicializa tmr0 desligado
    T016BIT     = 0;    //8 bit timer(1) 16 bit timer(0)
    T0CS        = 0;    //base de tempo do ciclo de instrução (0,2 us)
    PSA         = 0;    //habilita prescaler
    T0PS2:T0PS0 = 111;  //prescaler 1:256 (base de tempo em 51,2 us), para 8 bits, valor maximo de contagem = 13,1072 ms
    TMR0IF      = 0;    //limpa a flag do timer0 
}
void setTMR1(){
    //configurar tmr1(registrador T1CON)
    /* Para 8 bits:
     * 409,6u   = 409,6u - base desejada
     * 256      = x
     * x = [(409,6u - base desejada)*256]/409,6u
     * 
     * Para 16 bits:
     * 104,86m  = 104,86m - base desejada
     * 65536    = x
     * x = [(104,86m - base desejada)*65536]/104,86m     
     */
    RD16            = 1;    //16 bit timer(1) 8 bit timer(0)
    T1RUN           = 0;    //clock externo
    T1CKPS1:T1CKPS0 = 11;   //prescale 1:8 (base de tempo em 1,6 us)
                            //valor máximo de contagem 8 bits = 409,6 us
                            //valor máximo de contagem 16 bits = 104,86 ms
    T1OSCEN         = 0;    //desliga oscilador do timer1
    T1SYNC          = 0;    //quando TMR1CS = 0, bit ignorado
    TMR1CS          = 0;    //clock Fosc/4
    TMR1ON          = 0;    //inicializa tmr1 desligado
}

#include "mainusergui.h"
#include "ui_mainusergui.h"
#include <QSerialPort>      // Comunicacion por el puerto serie
#include <QSerialPortInfo>  // Comunicacion por el puerto serie
#include <QMessageBox>      // Se deben incluir cabeceras a los componentes que se vayan a crear en la clase
#include <QVariant>
// y que no estén incluidos en el interfaz gráfico. En este caso, la ventana de PopUp <QMessageBox>
// que se muestra al recibir un PING de respuesta

#include<stdint.h>      // Cabecera para usar tipos de enteros con tamaño
#include<stdbool.h>     // Cabecera para usar booleanos

MainUserGUI::MainUserGUI(QWidget *parent) :  // Constructor de la clase
    QWidget(parent),
    ui(new Ui::MainUserGUI)               // Indica que guipanel.ui es el interfaz grafico de la clase
  , transactionCount(0)
{
    ui->setupUi(this);                // Conecta la clase con su interfaz gráfico.
    setWindowTitle(tr("Interfaz de Control TIVA")); // Título de la ventana

    // Inicializa la lista de puertos serie
    ui->serialPortComboBox->clear(); // Vacía de componentes la comboBox
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // La descripción realiza un FILTRADO que  nos permite que SOLO aparezcan los interfaces tipo USB serial con el identificador de fabricante
        // y producto de la TIVA
        // NOTA!!: SI QUIERES REUTILIZAR ESTE CODIGO PARA OTRO CONVERSOR USB-Serie, cambia los valores VENDOR y PRODUCT por los correspondientes
        // o cambia la condicion por "if (true) para listar todos los puertos serie"
        if ((info.vendorIdentifier()==0x1CBE) && (info.productIdentifier()==0x0002))
        {
            ui->serialPortComboBox->addItem(info.portName());
        }
    }

    //Inicializa algunos controles
    ui->serialPortComboBox->setFocus();   // Componente del GUI seleccionado de inicio
    ui->pingButton->setEnabled(false);    // Se deshabilita el botón de ping del interfaz gráfico, hasta que

    //Inicializa la ventana de PopUp que se muestra cuando llega la respuesta al PING
    ventanaPopUp.setIcon(QMessageBox::Information);
    ventanaPopUp.setText(tr("Status: RESPUESTA A PING RECIBIDA")); //Este es el texto que muestra la ventana
    ventanaPopUp.setStandardButtons(QMessageBox::Ok);
    ventanaPopUp.setWindowTitle(tr("Evento"));
    ventanaPopUp.setParent(this,Qt::Popup);

    //Conexion de signals de los widgets del interfaz con slots propios de este objeto
    connect(ui->rojo,SIGNAL(toggled(bool)),this,SLOT(cambiaLEDs()));
    connect(ui->verde,SIGNAL(toggled(bool)),this,SLOT(cambiaLEDs()));
    connect(ui->azul,SIGNAL(toggled(bool)),this,SLOT(cambiaLEDs()));

    //Conectamos Slots del objeto "Tiva" con Slots de nuestra aplicacion (o con widgets)
    connect(&tiva,SIGNAL(statusChanged(int,QString)),this,SLOT(tivaStatusChanged(int,QString)));
    connect(&tiva,SIGNAL(messageReceived(uint8_t,QByteArray)),this,SLOT(messageReceived(uint8_t,QByteArray)));

    ui->led_der->setChecked(false);
    ui->led_izq->setChecked(false);

    ui->Muestreo->setVisible(false);
    ui->Frecuencia->setVisible(false);

    //inicialización gráfica
    ui->Grafica->setTitle("Muestreo de señales"); // Titulo de la grafica
    ui->Grafica->setAxisTitle(QwtPlot::xBottom, "Tiempo"); // Etiqueta eje X de coordenadas
    ui->Grafica->setAxisTitle(QwtPlot::yLeft, "Tensión (V)");    // Etiqueta eje Y de coordenadas
    //ui->Grafica->axisAutoScale(true); // Con Autoescala
    ui->Grafica->setAxisScale(QwtPlot::yLeft, 0, 3.3); // Escala fija
    ui->Grafica->setAxisScale(QwtPlot::xBottom,0,512.0);

    for(int i=0;i<6;i++)
    {
        Channels[i] = new QwtPlotCurve();
        Channels[i]->attach(ui->Grafica);
    }

    for(int i=0;i<512;i++)
    {
        xVal[i]=i;
        yVal[0][i]=0;
        yVal[1][i]=0;
        yVal[2][i]=0;
        yVal[3][i]=0;
        yVal[4][i]=0;
        yVal[5][i]=0;
    }

    Channels[0]->setRawSamples(xVal,yVal[0],512);
    Channels[1]->setRawSamples(xVal,yVal[1],512);
    Channels[2]->setRawSamples(xVal,yVal[2],512);
    Channels[3]->setRawSamples(xVal,yVal[3],512);
    Channels[4]->setRawSamples(xVal,yVal[4],512);
    Channels[5]->setRawSamples(xVal,yVal[5],512);

    Channels[0]->setPen(QPen(Qt::red)); // Color de la curva
    Channels[1]->setPen(QPen(Qt::cyan));
    Channels[2]->setPen(QPen(Qt::yellow));
    Channels[3]->setPen(QPen(Qt::green));
    Channels[4]->setPen(QPen(Qt::magenta));
    Channels[5]->setPen(QPen(Qt::blue));

    m_Grid = new QwtPlotGrid();        // Rejilla de puntos
    m_Grid->attach(ui->Grafica);       // que se asocia al objeto QwtPl
    //m_Grid->setPen(QPen(Qt::white)); //Cambio de color de la rejilla
    ui->Grafica->setAutoReplot(false); //Desactiva el autoreplot (mejora la eficiencia)
    ui->Grafica->setVisible(false);
    posicion=0;
}

MainUserGUI::~MainUserGUI() // Destructor de la clase
{
    delete ui;   // Borra el interfaz gráfico asociado a la clase
}

void MainUserGUI::on_serialPortComboBox_currentIndexChanged(const QString &arg1)
{
    activateRunButton();
}

// Funcion auxiliar de procesamiento de errores de comunicación
void MainUserGUI::processError(const QString &s)
{
    activateRunButton(); // Activa el botón RUN
    // Muestra en la etiqueta de estado la razón del error (notese la forma de pasar argumentos a la cadena de texto)
    ui->statusLabel->setText(tr("Status: Not running, %1.").arg(s));
}

// Funcion de habilitacion del boton de inicio/conexion
void MainUserGUI::activateRunButton()
{
    ui->runButton->setEnabled(true);
}

//FUNCIONES
void MainUserGUI::resetGrafica()
{
    posicion=0;
    for(int i=0;i<512;i++)
    {
        xVal[i]=i;
        yVal[0][i]=0;
        yVal[1][i]=0;
        yVal[2][i]=0;
        yVal[3][i]=0;
        yVal[4][i]=0;
        yVal[5][i]=0;
    }
    ui->Grafica->replot();
}

// SLOT asociada a pulsación del botón RUN
void MainUserGUI::on_runButton_clicked()
{
    //Intenta arrancar la comunicacion con la TIVA
    tiva.startRemoteLink( ui->serialPortComboBox->currentText());
}



//Slots Asociado al boton que limpia los mensajes del interfaz
void MainUserGUI::on_pushButton_clicked()
{
    ui->statusLabel->setText(tr(""));
}

void MainUserGUI::on_colorWheel_colorChanged(const QColor &arg1)
{
    //Poner aqui el codigo para pedirle al objeto "tiva" que envie el mensaje correspondiente
    MESSAGE_COLOR_PARAMETER parametro;
    parametro.red = arg1.red();
    parametro.green = arg1.green();
    parametro.blue = arg1.blue();
    tiva.sendMessage(MESSAGE_COLOR,&parametro,sizeof(parametro));
}

void MainUserGUI::on_Knob_valueChanged(double value)
{
    //Este mensaje se envia "mal" intencionadamente (no se corrsponde con lo que el microcontrolador pretende recibir)
    MESSAGE_LED_PWM_BRIGHTNESS_PARAMETER parametro;
    parametro.rIntensity=value;
    tiva.sendMessage(MESSAGE_LED_PWM_BRIGHTNESS,QByteArray::fromRawData((char *)&parametro,sizeof(parametro)));
}

void MainUserGUI::on_ADCButton_clicked()
{
    tiva.sendMessage(MESSAGE_ADC_SAMPLE,NULL,0);
}

void MainUserGUI::on_pingButton_clicked()
{
    tiva.sendMessage(MESSAGE_PING,NULL,0);
}

void MainUserGUI::cambiaLEDs(void)
{

    MESSAGE_LED_GPIO_PARAMETER parameter;

    parameter.leds.red=ui->rojo->isChecked();
    parameter.leds.green=ui->verde->isChecked();
    parameter.leds.blue=ui->azul->isChecked();

    tiva.sendMessage(MESSAGE_LED_GPIO,QByteArray::fromRawData((char *)&parameter,sizeof(parameter)));
}



//**** Slot asociado a la recepción de mensajes desde la TIVA ********/
//Está conectado a una señale generada por el objeto TIVA de clase QTivaRPC (se conecta en el constructor de la ventana, más arriba en este fichero))
//Se pueden añadir los que casos que quieran para completar la aplicación

void MainUserGUI::messageReceived(uint8_t message_type, QByteArray datos)
{
    switch(message_type) // Segun el comando tengo que hacer cosas distintas
    {
        /** A PARTIR AQUI ES DONDE SE DEBEN AÑADIR NUEVAS RESPUESTAS ANTE LOS COMANDOS QUE SE ENVIEN DESDE LA TIVA **/
        case MESSAGE_PING:
        {   //Recepcion de la respuesta al ping desde la TIVA
            // Algunos comandos no tiene parametros --> No se extraen
            ventanaPopUp.setStyleSheet("background-color: lightgrey");
            ventanaPopUp.setModal(true); //CAMBIO: Se sustituye la llamada a exec(...) por estas dos.
            ventanaPopUp.show();
        }
        break;

        case MESSAGE_REJECTED:
        {   //Recepcion del mensaje MESSAGE_REJECTED (La tiva ha rechazado un mensaje que le enviamos previamente)
            MESSAGE_REJECTED_PARAMETER parametro;
            if (check_and_extract_command_param(datos.data(), datos.size(), &parametro, sizeof(parametro))>0)
            {
                // Muestra en una etiqueta (statuslabel) del GUI el mensaje
            ui->statusLabel->setText(tr("Status: Comando rechazado,%1").arg(parametro.command));
            }
            else
            {
                ui->statusLabel->setText(tr("Status: MSG %1, recibí %2 B y esperaba %3").arg(message_type).arg(datos.size()).arg(sizeof(parametro)));
            }
        }
        break;

        case MESSAGE_ADC_SAMPLE:
        {    // Este caso trata la recepcion de datos del ADC desde la TIVA
            MESSAGE_ADC_SAMPLE_PARAMETER parametro;
            if (check_and_extract_command_param(datos.data(), datos.size(), &parametro, sizeof(parametro))>0)
            {
                ui->lcdCh1->display(((double)parametro.chan1)*3.3/4096.0);
                ui->lcdCh2->display(((double)parametro.chan2)*3.3/4096.0);
                ui->lcdCh3->display(((double)parametro.chan3)*3.3/4096.0);
                ui->lcdCh4->display(((double)parametro.chan4)*3.3/4096.0);
                ui->lcdCh5->display(((double)parametro.chan5)*3.3/4096.0);
                ui->lcdCh6->display(((double)parametro.chan6)*3.3/4096.0);
                if(ui->comboBox_3->currentIndex()==3)
                {
                    if(posicion<512)
                    {
                        yVal[0][posicion]=((double)parametro.chan1)*3.3/4096.0;
                        yVal[1][posicion]=((double)parametro.chan2)*3.3/4096.0;
                        yVal[2][posicion]=((double)parametro.chan3)*3.3/4096.0;
                        yVal[3][posicion]=((double)parametro.chan4)*3.3/4096.0;
                        yVal[4][posicion]=((double)parametro.chan5)*3.3/4096.0;
                        yVal[5][posicion]=((double)parametro.chan6)*3.3/4096.0;
                        posicion++;
                    }else
                    {
                        for(int i=0;i<511;i++)
                        {
                            yVal[0][i]=yVal[0][i+1];
                            yVal[1][i]=yVal[1][i+1];
                            yVal[2][i]=yVal[2][i+1];
                            yVal[3][i]=yVal[3][i+1];
                            yVal[4][i]=yVal[4][i+1];
                            yVal[5][i]=yVal[5][i+1];
                        }
                        yVal[0][511]=((double)parametro.chan1)*3.3/4096.0;
                        yVal[1][511]=((double)parametro.chan2)*3.3/4096.0;
                        yVal[2][511]=((double)parametro.chan3)*3.3/4096.0;
                        yVal[3][511]=((double)parametro.chan4)*3.3/4096.0;
                        yVal[4][511]=((double)parametro.chan5)*3.3/4096.0;
                        yVal[5][511]=((double)parametro.chan6)*3.3/4096.0;
                    }
                    ui->Grafica->replot();
                }
            }
            else
            {   //Si el tamanho de los datos no es correcto emito la senhal statusChanged(...) para reportar un error
                ui->statusLabel->setText(tr("Status: MSG %1, recibí %2 B y esperaba %3").arg(message_type).arg(datos.size()).arg(sizeof(parametro)));
            }
        }
        break;

        case MESSAGE_BUTTON:
        {   //Recepcion de la respuesta al ping desde la TIVA
            MESSAGE_BUTTON_PARAMETER parametro;
            if (check_and_extract_command_param(datos.data(), datos.size(), &parametro, sizeof(parametro))>0)
            {
                ui->led_izq->setChecked(parametro.left_button);
                ui->led_der->setChecked(parametro.right_button);
            }else
            {
                 ui->statusLabel->setText(tr("Status: MSG %1, recibí %2 B y esperaba %3").arg(message_type).arg(datos.size()).arg(sizeof(parametro)));
            }
        }
        break;

        case MESSAGE_LED_PWM_BRIGHTNESS:
        {
            MESSAGE_LED_PWM_BRIGHTNESS_PARAMETER parametro;
            if (check_and_extract_command_param(datos.data(), datos.size(), &parametro, sizeof(parametro))>0)
            {
               ui->Knob->setValue(parametro.rIntensity);
            }else
            {
                 ui->statusLabel->setText(tr("Status: MSG %1, recibí %2 B y esperaba %3").arg(message_type).arg(datos.size()).arg(sizeof(parametro)));
            }
        }
        break;


        default:
            //Notifico que ha llegado un tipo de mensaje desconocido
            ui->statusLabel->setText(tr("Status: Recibido mensaje desconocido %1").arg(message_type));
        break; //Innecesario
    }
}

//Este Slot lo hemos conectado con la señal statusChange de la TIVA, que se activa cuando
//El puerto se conecta o se desconecta y cuando se producen determinados errores.
//Esta función lo que hace es procesar dichos eventos
void MainUserGUI::tivaStatusChanged(int status,QString message)
{
    switch (status)
    {
        case TivaRemoteLink::TivaConnected:

            //Caso conectado..
            // Deshabilito el boton de conectar
            ui->runButton->setEnabled(false);

            // Se indica que se ha realizado la conexión en la etiqueta 'statusLabel'
            ui->statusLabel->setText(tr("Ejecucion, conectado al puerto %1.").arg(ui->serialPortComboBox->currentText()));

            //   Y se habilitan los controles deshabilitados
            ui->pingButton->setEnabled(true);

        break;

        case TivaRemoteLink::TivaIsDisconnected:
            //Caso desconectado..
            // Rehabilito el boton de conectar
             ui->runButton->setEnabled(false);
             ui->statusLabel->setText(message);
        break;
        case TivaRemoteLink::FragmentedPacketError:
        case TivaRemoteLink::CRCorStuffError:
            //Errores detectados en la recepcion de paquetes
            ui->statusLabel->setText(message);
        break;
        default:
            //Otros errores (por ejemplo, abriendo el puerto)
            processError(tiva.getLastErrorMessage());
    }
}




void MainUserGUI::on_comboBox_currentIndexChanged(int index)
{
    MESSAGE_MODE_PARAMETER parametro;
    parametro.index=index;
    tiva.sendMessage(MESSAGE_MODE,QByteArray::fromRawData((char *)&parametro,sizeof(parametro)));
}

void MainUserGUI::on_pushButton_2_clicked()
{

    tiva.sendMessage(MESSAGE_BUTTON,NULL,0);
}


void MainUserGUI::on_comboBox_2_currentIndexChanged(int index)
{
    MESSAGE_BUTTON_MODE_PARAMETER parametro;
    if(index)//MODO AUTO
    {
        ui->pushButton_2->setEnabled(false);
        parametro.mode=true;

    }else//MODO SONDEO
    {
        ui->pushButton_2->setEnabled(true);
        parametro.mode=false;
    }
    tiva.sendMessage(MESSAGE_BUTTON_MODE,QByteArray::fromRawData((char *)&parametro,sizeof(parametro)));
}

void MainUserGUI::on_comboBox_3_currentIndexChanged(int index)
{
    MESSAGE_ADC_MODE_PARAMETER parametro;
    parametro.index=index;
    if(index==0)
    {
        ui->ADCButton->setVisible(true);
    }else
    {
        ui->ADCButton->setVisible(false);
    }

    if(index==3)
    {
        resetGrafica();
        ui->Muestreo->setVisible(true);
        ui->Frecuencia->setVisible(true);
        ui->Grafica->setVisible(true);
    }else
    {
        ui->Muestreo->setVisible(false);
        ui->Frecuencia->setVisible(false);
        ui->Grafica->setVisible(false);
        ui->Muestreo->setChecked(false);
    }

    tiva.sendMessage(MESSAGE_ADC_MODE,QByteArray::fromRawData((char *)&parametro,sizeof(parametro)));
}

void MainUserGUI::on_factor_promediado_currentIndexChanged(int index)
{
    MESSAGE_FACTOR_PARAMETER parametro;
    parametro.factor=ui->factor_promediado->currentText().toInt();
    tiva.sendMessage(MESSAGE_FACTOR,QByteArray::fromRawData((char *)&parametro,sizeof(parametro)));
}

void MainUserGUI::on_Muestreo_toggled(bool checked)
{
    MESSAGE_TIMER_ADC_PARAMETER parametro;
    parametro.on=checked;
    parametro.frecuencia=ui->Frecuencia->value();
    if(checked)
    {
       resetGrafica();
    }
    tiva.sendMessage(MESSAGE_TIMER_ADC,QByteArray::fromRawData((char *)&parametro,sizeof(parametro)));
}



void MainUserGUI::on_Frecuencia_valueChanged(double value)
{
    MESSAGE_TIMER_ADC_PARAMETER parametro;
    if(ui->Muestreo->isChecked())
    {
        parametro.frecuencia=value;
        parametro.on=true;
        tiva.sendMessage(MESSAGE_TIMER_ADC,QByteArray::fromRawData((char *)&parametro,sizeof(parametro)));
    }
}

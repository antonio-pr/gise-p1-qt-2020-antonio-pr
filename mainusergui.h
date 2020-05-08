#ifndef MAINUSERGUI_H
#define MAINUSERGUI_H

#include <QWidget>
#include <QtSerialPort/qserialport.h>
#include <QMessageBox>
#include "tiva_remotelink.h"
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include "remotelink_messages.h"

namespace Ui {
class MainUserGUI;
}

class MainUserGUI : public QWidget
{
    Q_OBJECT
    
public: 
    explicit MainUserGUI(QWidget *parent = 0);
    ~MainUserGUI();
    
private slots:
    // slots privados asociados mediante "connect" en el constructor
    void cambiaLEDs();
    void tivaStatusChanged(int status,QString message);
    void messageReceived(uint8_t type, QByteArray datos);

    //Slots asociados por nombre
    void on_runButton_clicked();    
    void on_serialPortComboBox_currentIndexChanged(const QString &arg1);
    void on_pushButton_clicked();    
    void on_colorWheel_colorChanged(const QColor &arg1);
    void on_Knob_valueChanged(double value);
    void on_ADCButton_clicked();
    void on_pingButton_clicked();


    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_2_clicked();

    void on_comboBox_2_currentIndexChanged(int index);

    void on_comboBox_3_currentIndexChanged(int index);

    void on_factor_promediado_currentIndexChanged(int index);

    void on_Muestreo_toggled(bool checked);


    void on_Frecuencia_valueChanged(double value);

    void on_Resolucion_currentIndexChanged(int index);

private:
    // funciones privadas
    void processError(const QString &s);
    void activateRunButton();
    void resetGrafica();

private:
    //Componentes privados
    Ui::MainUserGUI *ui;
    int transactionCount;
    QMessageBox ventanaPopUp;
    TivaRemoteLink tiva; //Objeto para gestionar la comunicacion de mensajes con el microcontrolador

    //SEMANA2: Para las graficas
    double xVal[512]; //valores eje X
    double yVal[6][512]; //valores ejes Y
    QwtPlotCurve *Channels[6]; //Curvas
    QwtPlotGrid  *m_Grid; //Cuadricula
    int posicion;
};

#endif // GUIPANEL_H

#include <iostream>
#include "../TaxiVis/KdTrip.hpp"
#include <QString>
#include <QTextStream>
#include <QStringList>
#include <QFile>
#include <fstream>
#include <iostream>
#include <QDebug>
#include <QDateTime>

using namespace std;

int main(int argc, char** argv){
    if(argc != 3){
        cout << "usage: ./csv2Binary <input file> <output binary file>" << endl;
        return -1;
    }

    //
    QString inputFilename  = argv[1];
    QString outputFilename = argv[2];
    QFile file(inputFilename);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        cout << "Could not open file: " << inputFilename.toStdString() << endl;
        return -1;
    }

    //ignore header
    QTextStream textStream(&file);
    QString header = textStream.readLine();
#ifdef DEBUG
    qDebug() << "Header: " << header;
    qDebug() << "   Num Fields " << header.split(",").size();
#endif

    //
    QFile outputFile(outputFilename);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        cout << "Could not write file: " << outputFilename.toStdString() << endl;
        return -1;
    }

    //
    QMap<QString,int> idsSoFar;
    QMap<QString,int> paymentsSoFar;

    int count = 1;

    while(!textStream.atEnd()){
        QString line;
        QStringList tokens;

        if(count % 10000 == 0){
            cout << "Processed " << count << " trips" << endl;
        }

        //
        line = textStream.readLine();
        tokens = line.split(',');

        //
        KdTrip::Trip myTrip = {0};
        QDateTime pickupDateTime  = QDateTime::fromString(tokens[0],"yyyy-MM-dd HH:mm:ss");
        QDateTime dropoffDateTime = QDateTime::fromString(tokens[1],"yyyy-MM-dd HH:mm:ss");


        myTrip.pickup_time  = pickupDateTime.toMSecsSinceEpoch()/1000;//KdTrip::Query::createTime(2011,5,1,8,1,55);
        myTrip.dropoff_time = dropoffDateTime.toMSecsSinceEpoch()/1000;//KdTrip::Query::createTime(2011,5,1,9,0,0);

        //
        myTrip.pickup_long   = tokens[2].toFloat();
        myTrip.pickup_lat    = tokens[3].toFloat();
        myTrip.dropoff_long  = tokens[4].toFloat();
        myTrip.dropoff_lat   = tokens[5].toFloat();

        //
        QString strID = tokens[6];
        uint16_t idTaxi = 0;
        if(idsSoFar.count(strID) > 0){
            idTaxi = idsSoFar[strID];
        }
        else{
            idTaxi = idsSoFar.size();
            idsSoFar[strID] = idTaxi;
        }

        //
        QString strPayment  = tokens[13];
        uint8_t paymentType = 0;
        if(paymentsSoFar.count(strPayment) > 0){
            paymentType = paymentsSoFar[strPayment];
        }
        else{
            paymentType = paymentsSoFar.size();
            paymentsSoFar[strPayment] = paymentType;
        }

        //
        myTrip.id_taxi       = idTaxi;
        myTrip.distance      = (uint16_t)(tokens[7].toFloat() * 100);
        myTrip.fare_amount   = (uint16_t)(tokens[8].toFloat() * 100);
        myTrip.surcharge     = (uint16_t)(tokens[9].toFloat() * 100);
        myTrip.mta_tax       = (uint16_t)(tokens[10].toFloat() * 100);
        myTrip.tip_amount    = (uint16_t)(tokens[11].toFloat() * 100);
        myTrip.tolls_amount  = (uint16_t)(tokens[12].toFloat() * 100);
        myTrip.payment_type  = paymentType;
        myTrip.passengers    = tokens[14].toUInt();
        //
        myTrip.field1        = tokens[15].toUInt();
        myTrip.field2        = tokens[16].toUInt();
        myTrip.field3        = tokens[17].toUInt();
        myTrip.field4        = tokens[18].toUInt();

        //
#ifdef DEBUG
        qDebug() << "Line:" << line;
        cout << "   Taxi ID "      << myTrip.id_taxi      << endl;
        cout << "   pickup_long "  << myTrip.pickup_long  << endl;
        cout << "   pickup_lat  "  << myTrip.pickup_lat   << endl;
        cout << "   dropoff_long " << myTrip.dropoff_long << endl;
        cout << "   dropoff_lat  " << myTrip.pickup_lat   << endl;
        cout << "   fare  " << myTrip.fare_amount   << endl;
        cout << "   distance  " << myTrip.distance   << endl;
        cout << "   field 1  " << myTrip.field1   << endl;
#endif

        //
        outputFile.write((char*)&myTrip,sizeof(KdTrip::Trip));

        //
        ++count;
    }
    outputFile.close();

    cout << "Wrote " << count << " trips" << endl;
    cout << "   Size of trip " << sizeof(KdTrip::Trip) << endl;
    cout << "   Total Size   " << count * sizeof(KdTrip::Trip) << endl;

    return 0;
}

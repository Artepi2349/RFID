import time
import pandas as pd
import serial
ser = serial.Serial('COM4', 9600)

xl1 = pd.read_excel('C:/Users/Artem V/PycharmProjects/pythonProject/ПредПроф/RFID/database.xlsx') #Путь к таблице
data = time.localtime(time.time())

UID_match = 0  # Проверка совпадения
data_match= 0  # Проверка срока работы метки
date1 = xl1.iloc[0, 2]
day1 = str(date1)[8:10]
month1 = str(date1)[5:7]
year1 = str(date1)[0:4]

date2 = xl1.iloc[1, 2]
day2 = str(date2)[8:10]
month2 = str(date2)[5:7]
year2 = str(date2)[0:4]

UID1 = xl1.iloc[0, 1]
UID2 = xl1.iloc[1, 1]

controlDate = 0
counter1 = 0

while True:

    if controlDate<data.tm_mday:
        controlDate = data.tm_mday
        counter1 = 0

    UID_match = 0
    data_match = 0
    UID = ser.readline().decode('utf-8')[:12]

    print(UID)

    if UID == UID1:
        UID_match = 1
        if data.tm_year < int(year1):
            data_match = 1
        elif data.tm_year == int(year1):
            if data.tm_mon < int(month1):
                data_match = 1
            elif data.tm_mon == int(month1):
                if data.tm_mday <= int(day1):
                    data_match = 1
    elif UID == UID2:
        UID_match = 1
        if data.tm_year < int(year2):
            data_match = 1
        elif data.tm_year == int(year2):
            if data.tm_mon < int(month2):
                data_match = 1
            elif data.tm_mon == int(month2):
                if data.tm_mday <= int(day2):
                    data_match = 1

    if UID_match == 1 and data_match == 1 and counter1 == 0:
        print('Доступ разрешён')
        counter1 += 1
        ser.write(b'granted')
    elif UID_match == 1 and data_match == 1 and counter1>0:
        print('Повторное использование запрещено')
        ser.write(b'secondusage')
    elif UID_match == 1 and data_match == 0:
        print('Срок использования карточки закончен')
        ser.write(b'dateout')
    else:
        print('Карточки нет в базе данных')
        ser.write(b'notfind')
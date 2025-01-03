import time
import pandas as pd
import serial
ser = serial.Serial('COM4', 9600)

xl1 = pd.read_excel('C:/Users/Artem V/PycharmProjects/pythonProject/ПредПроф/RFID /database.xlsx') #Путь к таблице
data = time.localtime(time.time())

UID_match = 0  # Проверка совпадения UID
data_match= 0  # Проверка срока работы метки

date1 = xl1.iloc[0, 2]
day1 = str(date1)[8:10]
month1 = str(date1)[5:7]
year1 = str(date1)[0:4]

date2 = xl1.iloc[1, 2]
day2 = str(date2)[8:10]
month2 = str(date2)[5:7]
year2 = str(date2)[0:4]

date3 = xl1.iloc[2, 2]
day3 = str(date3)[8:10]
month3 = str(date3)[5:7]
year3 = str(date3)[0:4]

UID1 = xl1.iloc[0, 1]
UID2 = xl1.iloc[1, 1]
UID3 = xl1.iloc[2, 1]

while True:
    UID_match = 0
    data_match = 0

    UID = ser.readline().decode('utf-8')[:12]

    print(UID)

    if UID == UID1:
        UID_match = 1
        if data.tm_year <= int(year1) and data.tm_mon <= int(month1) and data.tm_mday <= int(day1):
            data_match = 1
    elif UID == UID2:
        UID_match = 1
        if data.tm_year <= int(year2) and data.tm_mon <= int(month2) and data.tm_mday <= int(day2):
            data_match = 1
    elif UID == UID3:
        UID_match = 1
        if data.tm_year <= int(year3) and data.tm_mon <= int(month3) and data.tm_mday <= int(day3):
            data_match = 1

    if UID_match == 1 and data_match == 1:
        print('Доступ разрешён')
        ser.write(b'granted')

    elif UID_match == 1 and data_match == 0:
        print('Срок использования карточки закончен')
        ser.write(b'dateout')
    else:
        print('Карточки нет в базе данных')
        ser.write(b'notfind')


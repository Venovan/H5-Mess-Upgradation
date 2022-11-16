from asyncio.windows_events import NULL
from .models import Student, Meal
from .serializer import LoginSerializer, StudentSerializer
from rest_framework.decorators import api_view
from rest_framework.response import Response
from rest_framework import status
from datetime import datetime

today = str(datetime.now().date())
hours = datetime.now().hour


MEAL_TYPE = ''

if hours in range(7, 11):
    MEAL_TYPE = 'B'
elif hours in range(11, 16):
    MEAL_TYPE = 'L'
elif hours in range(16, 19):
    MEAL_TYPE = 'S'
elif hours in range(0, 23):
    MEAL_TYPE = 'D'
else:
    MEAL_TYPE = NULL


PIN_CODES = [NULL, NULL, NULL]
ROLL_WAITING = [NULL, NULL, NULL]



@api_view(['GET'])
def  login(request, rfid_pin):
    if request.method == 'GET':
        if len(rfid_pin) == 9:
            try:
                student = Student.objects.get(rollNumber=rfid_pin)
                try:
                    Meal.objects.get(student=student, type= MEAL_TYPE, date=datetime.now().date())
                    return Response(status=status.HTTP_208_ALREADY_REPORTED)
                except Meal.DoesNotExist:
                    meal = Meal(student=student, type = MEAL_TYPE, weight="", date=datetime.now().date())
                    meal.save()
                    return Response(status=status.HTTP_201_CREATED)
            except Student.DoesNotExist:
                return Response(status=status.HTTP_404_NOT_FOUND)


            # if request.method == 'GET':
            #     serializer = StudentSerializer(student)
            #     return Response(serializer.data)

        elif rfid_pin == "data":
            if (ROLL_WAITING[1] != NULL):
                try:
                    student = Student.objects.get(rollNumber=ROLL_WAITING[1])
                    try:
                        Meal.objects.get(student=student, type= MEAL_TYPE, date=datetime.now().date())
                        return Response(status=status.HTTP_208_ALREADY_REPORTED)
                    except Meal.DoesNotExist:
                        meal = Meal(student=student, type = MEAL_TYPE, weight="", date=datetime.now().date())
                        meal.save()
                        return Response(status=status.HTTP_201_CREATED)
                except Student.DoesNotExist:
                    return Response(status=status.HTTP_404_NOT_FOUND)
            else:
                return Response(status=status.HTTP_404_NOT_FOUND)
        
        elif len(rfid_pin) == 4:
            PIN_CODES[1] = rfid_pin
            return Response(status=status.HTTP_202_ACCEPTED)
    
    


        



@api_view(['GET', 'PATCH'])
def register(request, rfid_pin):
    if request.method == 'GET':
        if len(rfid_pin) == 4:
            PIN_CODES[0] = rfid_pin
            return Response(status=status.HTTP_202_ACCEPTED)
        elif len(rfid_pin) == 8:
            if ROLL_WAITING[0] != NULL:
                student = Student.objects.get(rollNumber=ROLL_WAITING[0])
                student.RFID = rfid_pin
                student.save()
                return Response(status=status.HTTP_423_LOCKED)
            else:
                return Response(status=status.HTTP_304_NOT_MODIFIED)




@api_view(['GET', 'POST'])
def app(request, call):
    if request.method == 'GET':
        if len(call) == 14:
            ROLL_WAITING[int(call[9])] = call[:9]
            return Response(status=status.HTTP_202_ACCEPTED)
        if len(call) == 9:
            try:
                student = Student.objects.get(rollNumber = call)
            except Student.DoesNotExist:
                return Response(status=status.HTTP_404_NOT_FOUND)
            serializer = StudentSerializer(student)
            return Response(serializer.data)
        print(call)
            
    elif request.method == "POST":  
        serializer = StudentSerializer(data=request.data)
        if serializer.is_valid():
            serializer.save()
            return Response(serializer.data, status=status.HTTP_201_CREATED)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

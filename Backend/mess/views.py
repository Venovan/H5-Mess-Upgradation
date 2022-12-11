from .models import Student, Meal, Menu, Announcement
from .serializer import LoginSerializer, StudentSerializer, MenuSerializer, NoticeSerializer
from rest_framework.decorators import api_view
from rest_framework import viewsets
from rest_framework.response import Response
from rest_framework import status
from datetime import datetime, timedelta
from django.db.models import Avg, Sum, Variance
import H5Mess.settings as settings
from rest_framework.parsers import FormParser, MultiPartParser
import numpy as np
import requests
import base64

today = str(datetime.now().date())
hours = datetime.now().hour


MEAL_TYPE = ''

if hours in range(7, 12):
    MEAL_TYPE = 'B'
elif hours in range(12, 16):
    MEAL_TYPE = 'L'
elif hours in range(16, 19):
    MEAL_TYPE = 'S'
elif hours in range(0, 23):
    MEAL_TYPE = 'D'
else:
    MEAL_TYPE = None


PIN_CODES = [None, None, None]
ROLL_WAITING = [None, None, None]


@api_view(['POST'])
def sso_login(request):
    access_code = request.data.get("access_code")
    print("ACCESS CODE", access_code)
    token_exchange_response = requests.post(url=settings.TOKEN_EXCHANGE_URL,
                                            headers={
                                                "Authorization": "Basic {}".format(base64.b64encode("{}:{}".format(settings.CLIENT_ID, settings.CLIENT_SECRET).encode('ascii')).decode('ascii')),
                                                "Content-Type": "application/x-www-form-urlencoded; charset=UTF-8",
                                            },
                                            data="code={}&redirect_uri={}&grant_type=authorization_code".format(access_code, settings.REDIRECT_URI),)
    token_exchanage = token_exchange_response.json()
    print("TOKEN EXCHANGE RESPONSE", token_exchanage)
    access_token = token_exchanage.get("access_token")
    resources_response = requests.get(url=settings.RESOURCES_URL, headers={
        "Authorization": "Bearer {}".format(access_token),
    },)
    resources = resources_response.json()
    print(resources)
    roll_number = resources.get("roll_number")
    name = "{} {}".format(resources.get("first_name"),
                          resources.get("last_name"))
    student, _ = Student.objects.get_or_create(
        name=name, rollNumber=roll_number)
    result = StudentSerializer(student, context={"request": request})
    return Response(result.data)


@api_view(['POST'])
def update(request):
    rollNumber = request.data.get("rollNumber")
    roomNumber = request.data.get("roomNumber")
    print(rollNumber, roomNumber)
    student = Student.objects.get(rollNumber=rollNumber)
    student.roomNumber = roomNumber
    if len(dict(request.FILES)) > 0 and "file" in dict(request.FILES).keys():
        print("CONTAINS IMAGE")
        student.photo = request.FILES["file"]
    student.save()
    return Response({
        "messge": "Updated successfully",
    }, status=status.HTTP_200_OK)


@api_view(['GET'])
def get_student(request):
    roll_number = request.headers.get("rollNumber")
    if Student.objects.filter(rollNumber=roll_number).exists():
        student = Student.objects.get(rollNumber=roll_number)
        result = StudentSerializer(student, context={"request": request})
        return Response(result.data)
    else:
        return Response({"message": "No student with the matching roll number"})


@api_view(['GET', 'PATCH'])
def register(request, rfid_pin):
    if request.method == 'GET':
        if len(rfid_pin) == 4:
            PIN_CODES[0] = rfid_pin
            return Response(status=status.HTTP_202_ACCEPTED)
        elif len(rfid_pin) == 8:
            if ROLL_WAITING[0] != None:
                student = Student.objects.get(rollNumber=ROLL_WAITING[0])
                student.RFID = rfid_pin
                student.save()
                ROLL_WAITING[0] == None
                return Response(status=status.HTTP_423_LOCKED)
            else:
                return Response(status=status.HTTP_304_NOT_MODIFIED)


@api_view(['GET'])
def login(request, rfid_pin):
    if request.method == 'GET':
        if len(rfid_pin) == 4:
            PIN_CODES[1] = rfid_pin
            return Response(status=status.HTTP_202_ACCEPTED)

        elif len(rfid_pin) == 8:
            try:
                student = Student.objects.get(RFID=rfid_pin)
                if (student.permission == 'NA'):
                    return Response(student.name, status=status.HTTP_403_FORBIDDEN)
                elif (Meal.objects.filter(student=student, type=MEAL_TYPE, date=datetime.now().date()).exists()):
                    return Response(student.name, status=status.HTTP_208_ALREADY_REPORTED)
                else:
                    meal = Meal(student=student, type=MEAL_TYPE,
                                weight="", date=datetime.now().date())
                    meal.save()
                    return Response(student.name, status=status.HTTP_200_OK)
            except Student.DoesNotExist:
                return Response(status=status.HTTP_404_NOT_FOUND)

        elif rfid_pin == "update":
            if (ROLL_WAITING[1] != None):
                roll = ROLL_WAITING[1]
                ROLL_WAITING[1] = None
                try:
                    student = Student.objects.get(rollNumber=roll)
                    if (student.permission == 'NA'):
                        return Response(student.name, status=status.HTTP_403_FORBIDDEN)
                    elif (Meal.objects.filter(student=student, type=MEAL_TYPE, date=datetime.now().date()).exists()):
                        return Response(student.name, status=status.HTTP_208_ALREADY_REPORTED)
                    else:
                        meal = Meal(student=student, type=MEAL_TYPE,
                                    weight="", date=datetime.now().date())
                        meal.save()
                        return Response(student.name, status=status.HTTP_201_CREATED)
                except Student.DoesNotExist:
                    return Response(status=status.HTTP_404_NOT_FOUND)
            else:
                return Response(status=status.HTTP_204_NO_CONTENT)


@api_view(['GET'])
def weight(request, rfid_pin):
    if request.method == 'GET':
        if len(rfid_pin) == 4:
            PIN_CODES[2] = rfid_pin
            return Response(status=status.HTTP_202_ACCEPTED)

        elif len(rfid_pin) == 8:
            try:
                student = Student.objects.get(rollNumber=rfid_pin)
                try:
                    meal = Meal.objects.get(
                        student=student, type=MEAL_TYPE, date=datetime.now().date())
                    meal.weight = None
                    meal.save()
                    return Response(status=status.HTTP_100_CONTINUE)
                except Meal.DoesNotExist:
                    return Response(status=status.HTTP_206_PARTIAL_CONTENT)
            except Student.DoesNotExist:
                return Response(status=status.HTTP_404_NOT_FOUND)

        elif rfid_pin == "update":
            if (ROLL_WAITING[2] != None):
                roll = ROLL_WAITING[2]
                ROLL_WAITING[2] = None
                print(roll)
                try:
                    student = Student.objects.get(rollNumber=roll)
                    try:
                        meal = Meal.objects.get(
                            student=student, type=MEAL_TYPE, date=datetime.now().date())
                        meal.weight = None
                        meal.save()
                        return Response(status=status.HTTP_100_CONTINUE)
                    except Meal.DoesNotExist:
                        return Response(status=status.HTTP_206_PARTIAL_CONTENT)
                except Student.DoesNotExist:
                    return Response(status=status.HTTP_404_NOT_FOUND)
            else:
                return Response(status=status.HTTP_204_NO_CONTENT)

        elif len(rfid_pin) == 5:
            weight = rfid_pin[len(rfid_pin) -
                              rfid_pin[::-1].find("0"):]  # in grams
            try:
                meal = Meal.objects.get(
                    student=student, type=MEAL_TYPE, date=datetime.now().date())
                meal.weight = weight
                meal.save()
                ROLL_WAITING[2] == None
            except:
                Response(status=status.HTTP_409_CONFLICT)


@api_view(['GET', 'POST'])
def app(request, call):
    if request.method == 'GET':
        if call == "menu":
            menu = Menu.objects.all()
            serializer = MenuSerializer(menu, many=True)
            return Response(serializer.data)

        elif call == "notices":
            notices = Announcement.objects.filter(display=True)
            serializer = NoticeSerializer(notices, many=True)
            return Response(serializer.data)

        elif len(call) == 9:
            try:
                student = Student.objects.get(rollNumber=call)
            except Student.DoesNotExist:
                return Response(status=status.HTTP_404_NOT_FOUND)
            serializer = StudentSerializer(
                student, context={"request": request})
            return Response(serializer.data)

    elif request.method == "POST":
        if call == "newStudent":
            serializer = StudentSerializer(data=request.data)
            if serializer.is_valid():
                serializer.save()
                return Response(serializer.data, status=status.HTTP_201_CREATED)
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

        elif call == "validate":
            if PIN_CODES[int(request.data.get("machine"))] == request.data.get("pincode"):
                ROLL_WAITING[int(request.data.get("machine"))
                             ] = request.data.get("rollNumber")
                return Response(status=status.HTTP_202_ACCEPTED)
            else:
                return Response(status=status.HTTP_406_NOT_ACCEPTABLE)


@api_view(['GET'])
def arena(request, call):
    if request.method == 'GET':
        if call == "dateTotal":
            return Response(avg(request.data.get("date"), request.data.get("type")))
        elif call == "dateAvg":
            return Response(average_day_waste(request.data.get("date"), request.data.get("type")))
        elif call == "dateVar":
            return Response(variance_date_waste(request.data.get("date"), request.data.get("type")))
        elif call == "movingAvg":
            end = request.data.get("end")
            end = datetime(
                int(end.split("/")[0]), int(end.split("/")[1]), int(end.split("/")[2]))
            start = end - timedelta(days=int(request.data.get("days")))
            return Response(moving_avg_waste(start, end, request.data.get("type")))

# OVERALL STATISTICS API CALLS


def average_day_waste(date, type=None):
    date = datetime(
        int(date.split("/")[0]), int(date.split("/")[1]), int(date.split("/")[2]))
    print(type)
    if type == None:
        return Meal.objects.filter(date=date).aggregate(Avg("weight"))
    else:
        return Meal.objects.filter(date=date, type=type).aggregate(Avg("weight"))


def avg(date, type=None):
    date = datetime(
        int(date.split("/")[0]), int(date.split("/")[1]), int(date.split("/")[2]))
    if type == None:
        ar = np.array(Meal.objects.filter(date=date).values_list(
            "weight", flat=True)).astype(int)
        print(ar)
        return Meal.objects.filter(date=date).aggregate(Sum("weight"))
    else:
        return Meal.objects.filter(date=date, type=type).aggregate(Sum("weight"))


def total_day_waste(date, type=None):
    date = datetime(
        int(date.split("/")[0]), int(date.split("/")[1]), int(date.split("/")[2]))
    print(type)
    if type == None:
        return Meal.objects.filter(date=date).aggregate(Sum("weight"))
    else:
        return Meal.objects.filter(date=date, type=type).aggregate(Sum("weight"))


def variance_date_waste(date, type=None):
    date = datetime(
        int(date.split("/")[0]), int(date.split("/")[1]), int(date.split("/")[2]))
    if type == None:
        return Meal.objects.filter(date=date).aggregate(Variance("weight"))
    else:
        return Meal.objects.filter(date=date, type=type).aggregate(Variance("weight"))


def moving_avg_waste(start, end, type=None):
    if type == None:
        return Meal.objects.filter(date__range=[start, end]).aggregate(Avg("weight"))
    else:
        return Meal.objects.filter(date__range=[start, end], type=type).aggregate(Avg("weight"))


# INDIVIDUAL STATISTICS API CALLS

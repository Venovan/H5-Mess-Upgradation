from .models import Student, Meal, Menu, Announcement
from .serializer import LoginSerializer, StudentSerializer, MenuSerializer, NoticeSerializer
from rest_framework.decorators import api_view
from rest_framework.response import Response
from rest_framework import status
from datetime import datetime, timedelta
from django.db.models import Avg, Sum, Variance
import H5Mess.settings as settings
import requests
import base64
import json

today = str(datetime.now().date())
hours = datetime.now().hour


MEAL_TYPE = ''

if hours in range(6, 12):
    MEAL_TYPE = 'B'
elif hours in range(12, 18):
    MEAL_TYPE = 'L'
elif hours in range(18, 0):
    MEAL_TYPE = 'S'
elif hours in range(0, 6):
    MEAL_TYPE = 'D'
else:
    MEAL_TYPE = None


PIN_CODES = [None, None, None]
ROLL_WAITING = [None, None, None]

@api_view(['POST'])
def sso_login(request):
    access_code = request.data.get("access_code")
    token_exchange_response = requests.post(url=settings.TOKEN_EXCHANGE_URL,
                                            headers={
                                                "Authorization": "Basic {}".format(base64.b64encode("{}:{}".format(settings.CLIENT_ID, settings.CLIENT_SECRET).encode('ascii')).decode('ascii')),
                                                "Content-Type": "application/x-www-form-urlencoded; charset=UTF-8",
                                            },
                                            data="code={}&redirect_uri={}&grant_type=authorization_code".format(access_code, settings.REDIRECT_URI),)
    token_exchanage = token_exchange_response.json()
    access_token = token_exchanage.get("access_token")
    resources_response = requests.get(url=settings.RESOURCES_URL, headers={
        "Authorization": "Bearer {}".format(access_token),
    },)
    resources = resources_response.json()
    roll_number = resources.get("roll_number")
    name = "{} {}".format(resources.get("first_name"),
                          resources.get("last_name"))
    student, _ = Student.objects.get_or_create(
        name=name, rollNumber=roll_number)
    result = StudentSerializer(student)
    return Response(result.data)


@api_view(['GET', 'PATCH'])
def register(request, rfid_pin):
    if request.method == 'GET':
        if rfid_pin == "pin":
            PIN_CODES[0] = request.GET["code"]
            return Response(status=status.HTTP_202_ACCEPTED)
        elif rfid_pin == "card":
            if ROLL_WAITING[0] != None:
                student = Student.objects.get(rollNumber=ROLL_WAITING[0])
                student.RFID = request.GET["rfid"]
                student.save()
                ROLL_WAITING[0] == None
                return Response(status=status.HTTP_423_LOCKED)
            else:
                return Response(status=status.HTTP_304_NOT_MODIFIED)


@api_view(['GET'])
def login(request, rfid_pin):
    if request.method == 'GET':
        if rfid_pin == "pin":
            PIN_CODES[1] = request.GET["code"]
            return Response(status=status.HTTP_202_ACCEPTED)

        elif rfid_pin == "recognise":
            try:
                RFID = request.GET["rfid"]
            except:
                RFID = None
            if RFID == None:
                RFID =  ROLL_WAITING[1]
                ROLL_WAITING[1] = None
            if RFID != None:
                try:
                    student = Student.objects.get(RFID=RFID)
                    if (student.permission == 'NA'):
                        return Response(student.name, status=status.HTTP_403_FORBIDDEN)
                    elif (Meal.objects.filter(student=student, type=MEAL_TYPE, date=datetime.now().date()).exists()):
                        return Response(student.name, status=status.HTTP_208_ALREADY_REPORTED)
                    else:
                        meal = Meal(student=student, type=MEAL_TYPE, weight=None, date=datetime.now().date())
                        meal.save()
                        return Response(student.name, status=status.HTTP_201_CREATED)
                except Student.DoesNotExist:
                    return Response(status=status.HTTP_404_NOT_FOUND)
            else:
                return Response(status=status.HTTP_204_NO_CONTENT)






@api_view(['GET'])
def weight(request, rfid_pin):
    if request.method == 'GET':
        if rfid_pin == "pin":
            PIN_CODES[2] = request.GET["code"]
            return Response(status=status.HTTP_202_ACCEPTED)

        elif rfid_pin == "recognise":
            try:
                RFID = request.GET["rfid"]
            except:
                RFID = None
            if RFID == None:
                RFID =  ROLL_WAITING[2]
                ROLL_WAITING[2] = None
            if RFID != None:
                if (Student.objects.filter(RFID=RFID).exists()):
                    try:
                        meal = Meal.objects.get(student__RFID=RFID, type=MEAL_TYPE, date=datetime.now().date())
                        if (meal.weight == None):
                            return Response(RFID, status=status.HTTP_202_ACCEPTED)
                        else:
                            return Response(Student.objects.get(RFID=RFID).name, status=status.HTTP_405_METHOD_NOT_ALLOWED)
                    except Meal.DoesNotExist:
                        return Response(Student.objects.get(RFID=RFID).name, status=status.HTTP_206_PARTIAL_CONTENT)
                else:
                    return Response(status=status.HTTP_404_NOT_FOUND)
            else:
                return Response(status=status.HTTP_204_NO_CONTENT)

        elif rfid_pin == "update":
            meal = Meal.objects.get(student__RFID=request.GET["rfid"], type=MEAL_TYPE, date=datetime.now().date())
            meal.weight = request.GET["weight"]
            meal.save()
            return Response(Student.objects.get(RFID-RFID).name, status=status.HTTP_423_LOCKED)
        

@api_view(['GET', 'POST'])
def app(request, call):
    if request.method == 'GET':
        if call == "validate":
            if PIN_CODES[int(request.data.get("machine"))] == request.data.get("code"):
                ROLL_WAITING[int(request.data.get("machine"))] = request.data.get("rfid")
                return Response(status=status.HTTP_202_ACCEPTED)
            else:
                return Response(status=status.HTTP_406_NOT_ACCEPTABLE)

        elif call == "menu":
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
            serializer = StudentSerializer(student)
            return Response(serializer.data)

    elif request.method == "POST":
        if call == "newStudent":
            serializer = StudentSerializer(data=request.data)
            if serializer.is_valid():
                serializer.save()
                return Response(serializer.data, status=status.HTTP_201_CREATED)
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


@api_view(['GET'])
def arena(request, call):
    if request.method == 'GET':
        upto = request.data.get("upto")
        interval = request.data.get("interval")
        upto = upto if upto != None else today
        interval = interval if interval != None else "0"
        end = datetime(int(upto.split("/")[0]), int(upto.split("/")[1]), int(upto.split("/")[2]))
        start = end - timedelta(days=int(interval))
        mealtype = request.data.get("type")
        student = request.data.get("rollNumber")
        operation = request.data.get("operation")
        N = request.data.get("N")
        if call == "dateTotal":
            return Response(total_day_waste(start, end, mealtype))
        elif call == "dateAvg":
            return Response(average_day_waste(start, end, mealtype))
        # elif call == "dateVar":
        #     return Response(variance_date_waste(start, end, mealtype))
        elif call == "movingAvg":
            return Response(moving_avg_waste(start, end, mealtype))
        elif call == "myStats":       #datewise wastage
            return Response(student_days_total(student, start, end, mealtype, operation))
        elif call == "percentile":    #percentile among low wastage
            return Response(percentile(student ,mealtype))
        elif call == "topN":   #leaderboard
            return Response(top_N_scorers(N, start, end, mealtype))



# OVERALL STATISTICS API CALLS

def total_day_waste(start, end, type=None):
    if type == None:
        return Meal.objects.filter(date__range=[start, end]).aggregate(Sum("weight"))
    else:
        return Meal.objects.filter(date__range=[start, end], type=type).aggregate(Sum("weight"))

def average_day_waste(start, end, type=None):
    if type == None:
        return Meal.objects.filter(date_range=[start, end]).aggregate(Avg("weight"))
    else:
        return Meal.objects.filter(date__range=[start, end], type=type).aggregate(Avg("weight"))

def variance_date_waste(start, end, type=None):
    if type == None:
        return Meal.objects.filter(date__range=[start, end]).aggregate(Variance("weight"))
    else:
        return Meal.objects.filter(date__range=[start, end], type=type).aggregate(Variance("weight"))


def moving_avg_waste(start, end, type=None):
    if type == None:
        return Meal.objects.filter(date__range=[start, end]).aggregate(Avg("weight"))
    else:
        return Meal.objects.filter(date__range=[start, end], type=type).aggregate(Avg("weight"))


# INDIVIDUAL STATISTICS API CALLS
 
#returns total food wasted for last x days upto given date "upto"
def student_days_total(RL, start, end, type=None, operation="Sum"):
    if RL == None:
        return status.HTTP_400_BAD_REQUEST
    if type==None:
        meals =  Meal.objects.filter(student__rollNumber=RL, date__range = [start, end])
        if operation == "Sum":
            return meals.aggregate(Sum("weight"))
        elif operation =="Avg":
            return meals.aggregate(Avg("weight"))
    else:
        meals =  Meal.objects.filter(student__rollNumber= RL, type=type, date__range = [start, end])
        if operation == "Sum":
            return meals.aggregate(Sum("weight"))
        elif operation =="Avg":
            return meals.aggregate(Avg("weight"))

def percentile(id,type=None):
    students = Meal.objects.all().values('student_id').annotate(Avg("weight")).order_by("weight__avg") 
     
    totalstudents = students.count()
   
    myavg = students.filter(student_id=id).values_list("weight__avg")
    studentswithmorewaste = students.filter(weight__avg__gte=myavg).count()

    percentile = (studentswithmorewaste)*100.0/totalstudents
    
    return percentile

def top_N_scorers(N, start, end, type=None):
    if type==None:
        leaders =  Meal.objects.filter(date__range=[start, end]).values('student__rollNumber', "student__name").annotate(Avg("weight")).order_by("weight__avg") 
    else:
        leaders = Meal.objects.filter(date__range=[start, end], type=type).values('student__rollNumber').annotate(Avg("weight")).order_by("weight__avg") 
    
    N = len(leaders) if N == None else min(len(leaders), int(N))
    
    for each in range(N):
        del leaders[each]["student__rollNumber"]
    
    return leaders[:N]

    
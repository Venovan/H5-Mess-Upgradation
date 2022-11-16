from asyncio.windows_events import NULL
from .models import Meal, Student
from rest_framework import serializers
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
elif hours in range(19, 23):
    MEAL_TYPE = 'D'
else:
    MEAL_TYPE = NULL


class StudentSerializer(serializers.ModelSerializer):
    class Meta:
        model = Student
        fields = ["name", "permission", "rollNumber", "roomNumber"]



class LoginSerializer(serializers.ModelSerializer):
    status = serializers.SerializerMethodField()

    class Meta:
        model = Student
        fields = ["name", "status"]

    def get_status(self, student):
        
        try:
            Meal.objects.get(student = student, date = today, type=MEAL_TYPE)
            status = "Taken"
        except Meal.DoesNotExist:
            status = "Allowed"

        if student.permission == "NA":
            status = "Not Allowed"
        return status



class RegisterSerializer(serializers.ModelSerializer):
    class Meta:
        model = Student
        fields = [""]
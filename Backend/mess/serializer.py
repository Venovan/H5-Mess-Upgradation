from .models import Meal, Student, Menu, Announcement
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
    MEAL_TYPE = None


class StudentSerializer(serializers.ModelSerializer):
    class Meta:
        model = Student
        fields = "__all__"


class LoginSerializer(serializers.ModelSerializer):
    status = serializers.SerializerMethodField()

    class Meta:
        model = Student
        fields = ["name", "status"]

    def get_status(self, student):

        try:
            Meal.objects.get(student=student, date=today, type=MEAL_TYPE)
            status = "Taken"
        except Meal.DoesNotExist:
            status = "Allowed"

        if student.permission == "NA":
            status = "Not Allowed"
        return status


class MenuSerializer(serializers.ModelSerializer):
    class Meta:
        model = Menu
        fields = "__all__"


class NoticeSerializer(serializers.ModelSerializer):
    class Meta:
        model = Announcement
        fields = "__all__"

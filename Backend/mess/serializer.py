from .models import Meal, Student, Menu, Announcement
from rest_framework import serializers
from datetime import datetime


def get_meal_type():
    hours = datetime.now().hour
    print(hours)
    if hours in range(6, 12):
        return 'B'
    elif hours in range(11, 17):
        return 'L'
    elif hours in range(16, 20):
        return 'S'
    elif hours in range(19, 24):
        return 'D'


class StudentSerializer(serializers.ModelSerializer):
    photo = serializers.SerializerMethodField()

    def get_photo(self, student):
        request = self.context.get('request')
        photo_url = student.photo.url
        return request.build_absolute_uri(photo_url)

    class Meta:
        model = Student
        fields = "__all__"


class MealSerializer(serializers.ModelSerializer):
    class Meta:
        model = Meal
        fields = "__all__"


class LoginSerializer(serializers.ModelSerializer):
    status = serializers.SerializerMethodField()

    class Meta:
        model = Student
        fields = ["name", "status"]

    def get_status(self, student):
        try:
            Meal.objects.get(
                student=student, date=datetime.today(), type=get_meal_type())
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

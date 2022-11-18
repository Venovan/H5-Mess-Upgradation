from django.contrib import admin
from .models import Student, Meal, Announcement
# Register your models here.

admin.site.register(Student)
admin.site.register(Meal)
admin.site.register(Announcement)

from django.contrib import admin
from .models import Student, Meal, Announcement, Menu
# Register your models here.

admin.site.register(Student)
admin.site.register(Meal)
admin.site.register(Announcement)
admin.site.register(Menu)
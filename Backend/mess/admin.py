from django.contrib import admin
from .models import Student, Meal, Announcement, Menu
# Register your models here.

class StudentAdmin(admin.ModelAdmin):
    list_display = ('rollNumber', 'name')
    search_fields = ['rollNumber', 'name']


# class MealAdmin(admin.ModelAdmin):
#     list_display = ('date', 'rollNumber')
#     search_fields = ['date', 'name']


admin.site.register(Student, StudentAdmin)
admin.site.register(Meal)
admin.site.register(Announcement)
admin.site.register(Menu)
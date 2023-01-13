from django.urls import path
from mess import views


urlpatterns = [
    path("verify/", views.verify_network),
    path("login/<str:rfid_pin>", views.login),
    path("register/<str:rfid_pin>", views.register),
    path("weight/<str:rfid_pin>", views.weight),
    path("app/<str:call>", views.app),
    path("stats/<str:call>", views.arena),
    path("fill/<str:call>", views.fill_data),
    path("download/", views.map_export),
    path("get_student/", views.get_student),
    path("update/", views.update),
    path("sso_login/", views.sso_login),
    path("summary/", views.day_summary),
    path("test_student/", views.create_test_users),
    path("test_meals/", views.create_test_meals),
    path("days_eaten/", views.day_details),
    path("day_data/", views.day_data),
    path("cancel/", views.cancel_request),
    path("test_login/", views.test_login),
    path("summary_average/", views.day_summary_average),
]

from django.urls import path
from mess import views


urlpatterns = [
    path("login/<str:rfid_pin>", views.login),
    path("register/<str:rfid_pin>", views.register),
    path("weight/<str:rfid_pin>", views.weight),
    path("app/<str:call>", views.app),
    path("stats/<str:call>", views.arena)
]


from django.urls import path
from mess import views
from django.conf.urls.static import static
from django.conf import settings
from django.conf.urls.static import static


urlpatterns = [
    path("plate/<str:rfid_pin>", views.login),
    path("register/<str:rfid_pin>", views.register),
    path("weight/<str:rfid_pin>", views.weight),
    path("app/<str:call>", views.app),
    path("menu/", views.menu)
]

urlpatterns += static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)
from django.urls import path
from mess import views
from django.conf.urls.static import static
from django.conf import settings
 

urlpatterns = [
    path("login/<str:rfid_pin>", views.login),
    path("register/<str:rfid_pin>", views.register),
    path("app/<str:call>", views.app)
]

#urlpatterns += static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)
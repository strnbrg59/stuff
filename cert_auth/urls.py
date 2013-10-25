from django.conf.urls.defaults import patterns, include, url
from cert_auth import views

urlpatterns = patterns('',
        ('^generate_own_key_pair/$', views.generateOwnKeyPair),
        ('^accept_key_pair/$', views.acceptKeyPair),
        ('^show_own_certificate/$', views.showOwnCertificate),
        ('^sign_certificate/$', views.signCertificate),
)

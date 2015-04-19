from django.conf.urls import patterns, include, url
from views import main, search


urlpatterns = patterns('',
    (r'^$', main),
    (r'^search/$', search),

    # Examples:
    # url(r'^$', 'server.views.home', name='home'),
    # url(r'^blog/', include('blog.urls')),

    #url(r'^admin/', include(admin.site.urls)),
)

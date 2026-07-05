set(kritagreenhealpaintop_SOURCES
    greenheal.cpp
    kis_greenhealop.cpp
    kis_greenhealop_settings.cpp
    kis_greenhealop_settings_widget.cpp
    )


kis_add_library(kritagreenhealpaintop MODULE ${kritagreenhealpaintop_SOURCES})

target_link_libraries(kritagreenhealpaintop kritalibpaintop)

install(TARGETS kritagreenhealpaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-greenheal.png
    DESTINATION ${KDE_INSTALL_DATADIR}/krita/images)

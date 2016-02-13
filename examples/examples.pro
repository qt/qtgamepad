TEMPLATE = subdirs

SUBDIRS += simple

qtHaveModule(quick) {
    SUBDIRS += configureButtons quickGamepad

    qtHaveModule(widgets) {
        SUBDIRS += keyNavigation \
                   mouseItem
    }
}

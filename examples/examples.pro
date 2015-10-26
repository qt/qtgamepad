TEMPLATE = subdirs

SUBDIRS += simple \
    mouseItem

qtHaveModule(quick) {
    SUBDIRS += quickGamepad \
               keyNavigation \
               configureButtons
}

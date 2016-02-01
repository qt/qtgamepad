TEMPLATE = subdirs

SUBDIRS += simple

qtHaveModule(quick) {
    SUBDIRS += quickGamepad \
               keyNavigation \
               configureButtons \
               mouseItem
}

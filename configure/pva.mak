# QSRV/PVXS for PVA
ifdef PVXS_MAJOR_VERSION # prefer QSRV2 :)
$(APPNAME)_DBD += pvxsIoc.dbd
$(APPNAME)_LIBS += pvxsIoc pvxs
else
ifdef EPICS_QSRV_MAJOR_VERSION # fallback to QSRV1
$(APPNAME)_LIBS += qsrv
$(APPNAME)_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)
$(APPNAME)_DBD += PVAServerRegister.dbd
$(APPNAME)_DBD += qsrv.dbd
endif
endif


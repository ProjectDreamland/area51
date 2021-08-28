

@FOR %%I in (*.edl) DO @(
@echo Writing out %%~nI.txt
@EDLParser %%I > %%~nI.txt
)

@echo Success!
@pause

:END
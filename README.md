# DISCONTINUATION OF PROJECT #
This project will no longer be maintained by Intel.
Intel has ceased development and contributions including, but not limited to, maintenance, bug fixes, new releases, or updates, to this project.
Intel no longer accepts patches to this project.
# Provisioning Mode Manager
This component is used to manage the `RestrictionMode` property under U-Boot
environment variable using `u-boot-env-mgr`. This property determines the
filtering mechanism which will be applied while executig IPMI commands over
system interface.

## Design
BMC will restrict the execution of commands based on the following restriction
modes
* `Provisioning` - Allows all command execution through system interface
* `ProvisionedHostWhitelist` - Only whitelist commands will be executed after
  `CoreBiosDone` signal
* `ProvisionedHostDisabled` - No commands are allowed after `CoreBiosDone`
  signal

This component will expose the restriction modes through `RestrictionMode`
property under `xyz.openbmc_project.Control.Security.RestrictionMode` interface.
Also, a redfish event will be logged for any change in restriction mode.
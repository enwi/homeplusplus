import { DeviceRoutingModule } from './device-routing.module';

describe('DeviceRoutingModule', () => {
  let deviceRoutingModule: DeviceRoutingModule;

  beforeEach(() => {
    deviceRoutingModule = new DeviceRoutingModule();
  });

  it('should create an instance', () => {
    expect(deviceRoutingModule).toBeTruthy();
  });
});

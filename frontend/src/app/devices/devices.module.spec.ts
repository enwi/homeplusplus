import { DevicesModule } from './devices.module';

describe('DevicesModule', () => {
  let devicesModule: DevicesModule;

  beforeEach(() => {
    devicesModule = new DevicesModule();
  });

  it('should create an instance', () => {
    expect(devicesModule).toBeTruthy();
  });
});

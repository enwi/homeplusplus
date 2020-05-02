import { HueModule } from './hue.module';

describe('HueModule', () => {
  let hueModule: HueModule;

  beforeEach(() => {
    hueModule = new HueModule();
  });

  it('should create an instance', () => {
    expect(hueModule).toBeTruthy();
  });
});

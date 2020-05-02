import { RemoteSocketModule } from './remotesocket.module';

describe('RemotesocketModule', () => {
  let remotesocketModule: RemoteSocketModule;

  beforeEach(() => {
    remotesocketModule = new RemoteSocketModule();
  });

  it('should create an instance', () => {
    expect(remotesocketModule).toBeTruthy();
  });
});

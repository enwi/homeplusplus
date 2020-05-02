import { DeleteDialogModule } from './delete-dialog.module';

describe('DeleteDialogModule', () => {
  let deleteDialogModule: DeleteDialogModule;

  beforeEach(() => {
    deleteDialogModule = new DeleteDialogModule();
  });

  it('should create an instance', () => {
    expect(deleteDialogModule).toBeTruthy();
  });
});

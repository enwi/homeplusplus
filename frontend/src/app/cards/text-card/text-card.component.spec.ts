import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TextCardComponent } from './text-card.component';

describe('TextCardComponent', () => {
  let component: TextCardComponent;
  let fixture: ComponentFixture<TextCardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ TextCardComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TextCardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});

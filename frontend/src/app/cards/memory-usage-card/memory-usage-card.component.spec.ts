import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MemoryUsageCardComponent } from './memory-usage-card.component';

describe('MemoryUsageCardComponent', () => {
  let component: MemoryUsageCardComponent;
  let fixture: ComponentFixture<MemoryUsageCardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ MemoryUsageCardComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MemoryUsageCardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});

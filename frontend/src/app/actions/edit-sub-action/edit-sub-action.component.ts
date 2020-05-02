import {Component, ComponentFactoryResolver, ComponentRef, EventEmitter, Input, OnChanges, OnInit, Output, SimpleChange, SimpleChanges, ViewChild} from '@angular/core';
import {FormBuilder, FormGroup, Validators} from '@angular/forms';

import {timeoutValidator} from '../../validators/timeout-validator';
import {SubAction} from '../action';
import {SubActionTypeService} from '../sub-actions/sub-action-type.service';

import {TypeForm, TypeFormInfo} from './type-form';
import {TypeFormDirective} from './type-form.directive';

@Component({
  selector: 'app-edit-sub-action',
  templateUrl: './edit-sub-action.component.html',
  styleUrls: ['./edit-sub-action.component.css']
})
export class EditSubActionComponent implements OnInit, OnChanges {
  @Input() subAction: SubAction;
  @Output() finish = new EventEmitter<SubAction>();
  @Output() cancel = new EventEmitter<void>();
  @ViewChild(TypeFormDirective, {static: false}) insertPoint: TypeFormDirective;

  types: Array<TypeFormInfo>;

  currentComponent: ComponentRef<TypeForm>;
  form: FormGroup;

  constructor(
      private componentFactoryResolver: ComponentFactoryResolver,
      private fb: FormBuilder, private typeService: SubActionTypeService) {}

  ngOnInit() {
    this.types = this.typeService.getEditTypes();
    const type = this.subAction ?
        this.typeService.getEditType(this.subAction.type) :
        undefined;
    this.form = this.fb.group({
      type: [type, Validators.required],
      timeout: [
        this.subAction && this.subAction.timeout ? this.subAction.timeout + '' :
                                                   '',
        timeoutValidator
      ],
      transition:
          [this.subAction && this.subAction.transition ?
               this.subAction.transition :
               false]
    });
    this.form.get('type').valueChanges.subscribe(
        value => this.onComponentChanged(value));

    this.onComponentChanged(type);
  }

  ngOnChanges(changes: SimpleChanges) {
    if ('subAction' in changes) {
      const changedSubAction = changes['subAction'].currentValue;
      this.onSubActionChanged(changedSubAction);
    }
  }

  onComponentChanged(value: TypeFormInfo) {
    const viewContainer = this.insertPoint.viewContainerRef;
    viewContainer.clear();
    if (value) {
      const componentFactory =
          this.componentFactoryResolver.resolveComponentFactory(value.type);

      this.currentComponent = viewContainer.createComponent(componentFactory);
      this.currentComponent.instance.setSubAction(this.subAction);
    }
  }

  onSubActionChanged(subAction: SubAction) {
    if (subAction && this.form) {
      const type = this.typeService.getEditType(subAction.type);
      if (type.typeId !== this.form.value.typeId) {
        this.form.get('type').setValue(type);
      }
      if (this.currentComponent) {
        this.currentComponent.instance.setSubAction(this.subAction);
      }
      this.form.get('timeout').setValue(subAction.timeout + '');
      this.form.get('transition').setValue(subAction.transition);
    }
  }

  clickFinish() {
    if (this.currentComponent.instance.isValid()) {
      const subAction = this.currentComponent.instance.getSubAction();
      subAction.type = this.form.value.type.typeId;
      subAction.timeout = Math.floor(this.form.value.timeout);
      subAction.transition = this.form.value.transition;
      this.finish.emit(subAction);
    }
  }
  clickCancel() {
    this.cancel.emit();
  }
}

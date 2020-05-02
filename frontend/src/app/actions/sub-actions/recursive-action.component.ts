import { Component, OnInit, EventEmitter } from '@angular/core';
import { SubActionView } from './sub-action-view';
import { SubAction, Action } from '../action';
import { RecursiveAction } from './sub-actions';
import { ActionService } from '../action.service';
import { Observable } from 'rxjs';

@Component({
  selector: 'app-recursive-action',
  templateUrl: './recursive-action.component.html',
  styleUrls: ['./recursive-action.component.css', './sub-action.css']
})
export class RecursiveActionComponent implements OnInit, SubActionView {

  subAction: RecursiveAction;
  action$: Observable<Action>;
  edit: EventEmitter<void>;
  delete: EventEmitter<void>;

  constructor(private actionService: ActionService) { }

  ngOnInit() {
  }

  initialize(subAction: SubAction, editAction: EventEmitter<void>, deleteAction: EventEmitter<void>) {
    this.subAction = subAction as RecursiveAction;
    this.action$ = this.actionService.getAction(this.subAction.actionId);
    this.edit = editAction;
    this.delete = deleteAction;
  }
}

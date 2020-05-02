import {Component, EventEmitter, Input, OnInit, Output, Type} from '@angular/core';
import {MatDialog} from '@angular/material/dialog';

import {DeleteDialog} from '../../delete-dialog/delete-dialog.component';
import {Action, SubAction} from '../action';
import {ActionService} from '../action.service';
import {SubActionTypeService} from '../sub-actions/sub-action-type.service';
import {SubActionView} from '../sub-actions/sub-action-view';

@Component({
  selector: 'app-sub-action-list',
  templateUrl: './sub-action-list.component.html',
  styleUrls: ['./sub-action-list.component.css']
})
export class SubActionListComponent implements OnInit {
  @Input() subActions: SubAction[];
  @Output() edit = new EventEmitter<[number, SubAction]>();
  @Output() delete = new EventEmitter<number>();


  editingSubAction: SubAction;
  editSubActionId = -1;

  // Diese Unteraktion wirklich löschen? Kann nicht rückgängig gemacht werden.
  private dialogText =
      $localize`Really delete this sub-action? This cannot be undone.`;

  constructor(
      private typeService: SubActionTypeService, private dialog: MatDialog) {}

  ngOnInit() {
    if (!this.subActions) {
      throw new Error('SubActionListComponent must be given sub actions');
    }
  }

  editSubAction(id: number) {
    this.editingSubAction = this.subActions[id];
    this.editSubActionId = id;
  }
  addSubAction() {
    this.editingSubAction = new SubAction();
    this.editSubActionId = -1;
  }
  finishSubAction(subAction: SubAction) {
    if (this.editSubActionId === -1) {
      this.subActions.push(subAction);
    } else {
      this.subActions[this.editSubActionId] = subAction;
    }
    this.editingSubAction = undefined;
    this.edit.emit(
        [this.editSubActionId, this.subActions[this.editSubActionId]]);
  }
  cancelSubAction() {
    this.editSubActionId = -1;
    this.editingSubAction = undefined;
  }
  deleteSubAction(id: number) {
    const dialogRef =
        this.dialog.open(DeleteDialog, {data: {content: this.dialogText}});
    dialogRef.afterClosed().subscribe(result => {
      if (result) {
        // Remove element from subActions
        this.subActions.splice(id, 1);
        this.delete.emit(id);
      }
    });
  }
  getViewType(type: number): Type<SubActionView> {
    return this.typeService.getViewType(type);
  }
}

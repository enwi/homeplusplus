import { Component, OnInit } from '@angular/core';
import { Observable } from 'rxjs';
import { Action } from '../action';
import { ActionService } from '../action.service';
import { Router } from '@angular/router';
import { scan } from 'rxjs/operators';

@Component({
  selector: 'app-action-list',
  templateUrl: './action-list.component.html',
  styleUrls: ['./action-list.component.scss']
})
export class ActionListComponent implements OnInit {
  actions$: Observable<Action[]>;

  constructor(private actionService: ActionService, private router: Router) { }

  ngOnInit() {
    this.getActions();
  }

  getActions(): void {
    this.actions$ = this.actionService.getActionsAsArray();
  }
  executeAction(id: number): void {
    this.actionService.executeAction(id);
  }
}

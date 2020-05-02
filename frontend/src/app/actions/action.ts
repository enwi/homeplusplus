export class SubAction {
  type: number;
  timeout: number;
  transition: boolean;
}

export class Action {
  id: number;
  name: string;
  icon: string;
  color: number;
  subActions: SubAction[];
  visible: boolean;
}
